# C reimplementation of the archives (`src/reimpl`)

`src/reimpl` is a C version of the two prebuilt archives: `lib/lib_no2_o3.a` (`init_no2_o3`, `calc_no2_o3`) and `lib/lib_zmod4xxx_cleaning.a` (`zmod4xxx_cleaning_run`). It was written from the AArch64 disassembly and from the `.rodata` of the archive objects. It keeps the public symbols and prototypes of `src/algos/no2_o3.h` and `src/algos/zmod4xxx_cleaning.h`.

It is opt-in. The default build still links the two archives. `-DUSE_REIMPL=ON` builds `libzmod4510.so` from these files instead. The tree does not record why it was written. Read "Open points" at the end before relying on it.

## Files

| File | Replaces (object in the archive) | Notes |
| --- | --- | --- |
| `no2_o3.c` | `no2_o3.c.o` | `init_no2_o3`, `init_no2_o3_internal`, `calc_no2_o3`, `calc_no2_o3_internal`, `no2_o3_ver` (1.0.1) |
| `no2_o3_nnet.c` | `no2_o3_NO2_nnet.c.o`, `no2_o3_O3_nnet.c.o`, `nnet_funcs.c.o` | `calc_no2_o3_NO2_nnet`, `calc_no2_o3_O3_nnet`. The layer helpers are `static` here |
| `no2_o3_data.h` | | Declarations of every table below |
| `no2_o3_data.c` | `.rodata` of `no2_o3.c.o` | Compensation coefficients, long-term variance start values, EPA breakpoints. Generated |
| `no2_o3_nnet_data.c` | `.rodata` of the two network objects | 12 weight arrays (6 layers, NO2 and O3). Generated |
| `zmod4xxx_cleaning.c` | `zmod4xxx_cleaning.c.o` | `zmod4xxx_cleaning_run` |

`no2_o3_data.c` and `no2_o3_nnet_data.c` say "Do not edit by hand". They come from [../decompiled/scripts/extract_rodata.py](../decompiled/scripts/extract_rodata.py), see "Regenerating the data". Floats are C99 hex literals, so the values are bit-exact.

## Build

`CMakeLists.txt` has `option(USE_REIMPL ... OFF)`. With it ON:

- the five `.c` files are added to `libzmod4510.so`;
- each of them is compiled with `-ffp-contract=off`;
- the link line is `m` only. `lib/lib_no2_o3.a` and `lib/lib_zmod4xxx_cleaning.a` are not linked.

`src/reimpl` is on the include path in both modes. `setup.py` passes `-DUSE_REIMPL=ON`, so `pip install .` builds the reimplementation. A CMake build that does not pass the flag still links the archives.

```bash
cmake -S . -B build -DUSE_REIMPL=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
```

`-ffp-contract=off` matters. `no2_o3.c` and `no2_o3_nnet.c` write every fused multiply-add the original has as an explicit `fmaf()`. If the compiler fused other operations, the results would no longer be bit-identical.

Checked on 2026-10-28 in a scratch directory, not in `build/`: Raspberry Pi 4, `/usr/bin/cc` (`gcc` on this host is 14.2.0), Release (`-O3`), `-DUSE_REIMPL=ON`. Both targets built. `libzmod4510.so` was 76 432 bytes and needs only `libm.so.6` and `libc.so.6`. It exports `init_no2_o3`, `init_no2_o3_internal`, `calc_no2_o3`, `calc_no2_o3_internal`, both `calc_*_nnet` functions, `zmod4xxx_cleaning_run`, `no2_o3_ver` and also all the `no2_o3_*` tables, because nothing hides them. The existing `build/libzmod4510.so` (78 224 bytes) is the archive variant and was not rebuilt. The CMake build was not repeated with `gcc-6`.

## What `calc_no2_o3` does, with the numbers

The pipeline is the one in [decompiled-no2-o3.md](decompiled-no2-o3.md). That page had no constants. The ones below are read from `no2_o3.c`, which was compared bit for bit with the archive (see "Verification"). Table contents are not copied here, see `no2_o3_data.c`.

| Step | What the code does |
| --- | --- |
| Resistance | MOx channels are the big-endian words at bytes 8, 18, 22, 30 of the ADC data. `adc <= mox_lr` gives 100. `adc >= mox_er` gives 1e12. Otherwise `config[0] * 1000 * (adc - lr) / (er - adc)`, limited to 100 … 1e12 |
| On-chip temperature term | `therm = 15.909091 + 0.00145791 * word0` (ADC bytes 0–1) |
| Calibration terms | `t_cal1 = (config[2]<<8 \| config[3]) / 655.36`, `t_cal2 = config[5] * 0.5` |
| Temperature used | `temperature_degc` if it is above −280 and below 1 000 000. Otherwise (so for the −300 of `src/main.c`, and for NaN) `((therm / t_cal1) - 1) / 0.0037218 + t_cal2 - 1.3` |
| Counter | `sample_counter` goes up by one per call and stops at `0xFFFFFFFF`. Calls with the counter at 50 or lower are warm-up |
| Smoothing | Warm-up: `rmoxs_smooth = rmox`. After that: `smooth *= (rmox / smooth) ^ 0.2212` |
| Compensation | `comp = (1 + dT*(p10 + dT*p20) + dh*(p01 + dh*p02 + dT*p11)) * log10(smooth)` with `dh = humidity - 50`, `dT = temperature - 20`, coefficients per channel |
| Long-term statistics | Warm-up: `logrmoxs_mean = comp`, the variance keeps its start value. After that: `alpha = 1 - exp(-1/counter)`, and from counter 144 001 on a fixed `1 - exp(-1/144000)`. Mean and variance are updated with `alpha`. The variance is floored per channel (0.00282, 0.00197, 0.00164, 0.00152) |
| Network input | 14 floats: humidity, temperature, then per channel compensated log, mean, variance. Only after warm-up |
| Scale | Both network outputs are multiplied by `1.0426015 ^ (int8_t) prod_data[9]`. Negative results become 0 |
| Filters | `x += (new - x) * c` with `c = 0.09516259`, `0.001665279`, `0.0002083116`. O3 keeps 1 min, 1 h and 8 h values, NO2 keeps 1 min and 1 h |
| AQI | Piecewise-linear between the EPA breakpoints in `no2_o3_data.c`. Below the first breakpoint `conc * aqi[0] / conc[0]`. Above the last one the last AQI value |
| `FAST_AQI` | The larger of O3 1 min and NO2 1 min AQI, rounded to nearest (ties away from zero) |
| `EPA_AQI` | The largest of O3 1 h, O3 8 h and NO2 1 h AQI, truncated |
| Return | Counter 50 or lower: `NO2_O3_STABILIZATION`. Then `NO2_O3_DAMAGE` if `rmox[3] < 250` or `therm > 61.555`. Otherwise `NO2_O3_OK` |

Points that follow from this and are easy to miss:

- The three filter constants equal `1 - exp(-1/N)` for N = 10, 600 and 4800 samples, to float precision. At one sample per 6 s (the 6000 ms sleep in `sensor_step`) that is 1 min, 1 h and 8 h. This is arithmetic on the constants, not a vendor statement.
- The networks and the concentration filters run only after warm-up. In the first 50 calls `O3_conc_ppb`, `NO2_conc_ppb`, `FAST_AQI` and `EPA_AQI` are the 0 that `init_no2_o3` left. The archive does the same in the comparison. It was not seen on a sensor.
- `DAMAGE` cannot come from `rmox[3]` being too high. The code compares it with a limit near 1e13, and the resistance is already limited to 1e12. A value between 100 and 250 does give `DAMAGE`.
- The `therm` limit is reached at a word-0 ADC value of about 31 300.
- In one branch of the `EPA_AQI` selection the original masks the integer with `0xFFF`, in the others with `0xFFFF`. `no2_o3.c` keeps both masks. AQI is never above 500, so the result is the same.
- `nnet_funcs_tanh` returns exactly ±1 outside ±82 and otherwise `(e - 1/e) / (e + 1/e)` with `e = exp(x)`, `exp` taken in double.
- The signed-byte exponent that [decompiled-no2-o3.md](decompiled-no2-o3.md) marks as inferred is what the comparison matches. `prod_data[9]` is random in every run of the compare program, so values above 127 were very likely among them (20 runs by default). The program does not record which values were drawn.
## `zmod4xxx_cleaning_run`

The steps, the register numbers and the missing timeout are in [decompiled-zmod4xxx-cleaning.md](decompiled-zmod4xxx-cleaning.md) and are not repeated here. `zmod4xxx_cleaning.c` follows that page step by step, including the built-in configuration (`0xC0`, `fb b4`, `20 04 20 04`, `03`, `01 c0 81 c8`), the 50 ms polls, the 30 completions of the last sequencer step, and the raw result of the final write as return value. The loops have no timeout, as in the archive.

## Differences from the archives

- `nnet_funcs_dotbias` and `nnet_funcs_tanh` were global symbols in the archive. In `no2_o3_nnet.c` they are `static`. `nnet_funcs_relu` and `nnet_funcs_safe_exp` do not exist in `src/reimpl` because no network calls them. Nothing in `src/` outside `src/reimpl` refers to any of the four.
- The weight, coefficient and breakpoint tables are global read-only symbols (`no2_o3_*`, declared in `no2_o3_data.h`). They were local to the archive objects.
- Only the AArch64 archives that are in `lib/` were used. The 32-bit ARM and other catalog archives were not compared.

## Verification

Everything was run on the Raspberry Pi 4 (aarch64) on 2026-10-28. None of it touches a sensor or writes into the repository. The scripts build into `${TMPDIR:-/tmp}/reimpl_compare*` and only read the archives in `lib/`.

**`test_script/reimpl_compare.sh [samples [runs]]`** (defaults 2000 and 20, `CC` selects the compiler). It compiles `no2_o3.c` and `no2_o3_nnet.c` with the public names changed to `re_*`, links them next to `lib/lib_no2_o3.a` and calls both with identical random inputs: random ADC words (some forced to 0 or 0xFFFF), random `config`, `mox_lr`, `mox_er` and `prod_data`, humidity 0–100 %, temperature −10…60 °C or −300 on one call in ten. After every call it compares the return value, the whole results struct and the whole handle with `memcmp`. The results struct is filled with `0xA5` first, so a field that only one side leaves unwritten shows up. It exits non-zero on any difference. `reimpl_compare.c` has a fixed seed, so a run is repeatable.

**`test_script/reimpl_compare_cleaning.sh`** does the same for the cleaning function against `lib/lib_zmod4xxx_cleaning.a` on a mock I2C bus. The compared trace is every read, write (register, length, bytes) and delay, plus the return value. It runs with the counter register at 0 and at 1, and for each of them with `fail_at` from 0 to 89. 0 means no failure, any other value makes that numbered read or write fail. A clean pass with the counter at 0 is 70 reads and writes, so every one of them gets a failure. With the counter at 1 the function stops after 1 read and returns `-8`.

| Run | Result |
| --- | --- |
| `reimpl_compare.sh` defaults, `gcc` 14.2, `-O2` | 40 000 samples, 0 differences |
| same with `CC=gcc-6` | 40 000 samples, 0 differences |
| `reimpl_compare.sh 3000 10` at `-O0` and at `-O3`, each with `gcc` and `gcc-6` | 30 000 samples each, 0 differences |
| `reimpl_compare.sh 150000 2` | 300 000 samples, 0 differences, about 18 s |
| `reimpl_compare_cleaning.sh`, `gcc` and `gcc-6` | 180 cases, 0 differences |
| `extract_rodata.py` run again on the archive | Both generated files identical to the ones in `src/reimpl` |

The default run never reaches the `counter > 144000` branch. Use `reimpl_compare.sh 150000 1` or more samples to cover it. A throwaway copy of the compare program that counted return codes showed: the default run returned `NO2_O3_OK` 28 294 times, `NO2_O3_STABILIZATION` 1000 times and `NO2_O3_DAMAGE` 10 706 times. A run of 150 000 samples in one handle returned 95 613, 50 and 54 337. The compare program in the repository prints only the totals.

Not covered:

- No real sensor data. Random ADC words give large jumps and many `DAMAGE` results, so the steady state of a real sensor is exercised less than the corner cases. All outputs are compared whatever the status is.
- NaN and infinite inputs.
- The cleaning function on a real module. The mock bus shows the same I2C traffic and return codes as the archive, nothing more. The function writes to the sensor's non-volatile memory, and a successful run on a module that was not cleaned before cannot be undone (see [decompiled-zmod4xxx-cleaning.md](decompiled-zmod4xxx-cleaning.md)).
- `no2_o3-example` built with `USE_REIMPL=ON` was not started on the sensor. The Python module from that build is installed in `.venv`. It was not started either. See [skills/python-module-installation.md](skills/python-module-installation.md).
- Both sides call the same `libm` (`exp`, `log10`, `pow`). A different `libm` was not tested. The archive depends on `libm` the same way.

## Regenerating the data

```bash
mkdir -p /tmp/regen/obj /tmp/regen/out && cd /tmp/regen/obj
ar x /home/comet/renesas_zmod4510/lib/lib_no2_o3.a
python3 /home/comet/renesas_zmod4510/decompiled/scripts/extract_rodata.py /tmp/regen/obj /tmp/regen/out
cmp /tmp/regen/out/no2_o3_data.c /home/comet/renesas_zmod4510/src/reimpl/no2_o3_data.c
cmp /tmp/regen/out/no2_o3_nnet_data.c /home/comet/renesas_zmod4510/src/reimpl/no2_o3_nnet_data.c
```

The script needs `aarch64-linux-gnu-objcopy`. Section offsets and layer sizes are written into the script, and it asserts the 0xD60-byte size of each network `.rodata`. It is only valid for the archive that is in `lib/` now. Extract outside the repository, as in [skills/debug-static-archive.md](skills/debug-static-archive.md).

## Open points

- **License.** `no2_o3.h` and `zmod4xxx_cleaning.h` say the implementation is proprietary Renesas software under the license they link. `src/reimpl` is derived from the disassembly of those archives and contains their coefficient and weight tables value for value. The files carry no license header. The repository root `LICENSE` is GPL-3 (see [architecture.md](architecture.md)). This tree does not say whether the Renesas license allows this, and the license text was not read for this note: the URL in the headers returns a PDF that could not be turned into text here. Check it before a `USE_REIMPL=ON` build is shared.
- **Earlier rule.** [libraries.md](libraries.md) and [skills/debug-static-archive.md](skills/debug-static-archive.md) say not to add a C replacement of either archive, and `.grok/skills/zmod4510/SKILL.md` repeats it. [../decompiled/README.md](../decompiled/README.md) says the same. Those texts were not changed when `src/reimpl` was added, so they contradict the tree. Only `libraries.md` got a note that points here. The other three still say a replacement must not be added. [decompiled-no2-o3.md](decompiled-no2-o3.md) and [decompiled-zmod4xxx-cleaning.md](decompiled-zmod4xxx-cleaning.md) also still say they are "not a replacement". Which rule stands is for the owner to decide.
- **Sensor test.** Nothing here has run on the sensor.
