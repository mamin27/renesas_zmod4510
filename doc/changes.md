# What changed

Short log of important work in this tree. How the code behaves now stays in the topic file linked from each entry. Append a new entry at the top when that behavior, the build, the libraries, or the test procedure changes.

## 2026-10-28 — Test script prints 10 readings and their mean

`test_script/zmod4510_i2c_test.py` takes 10 `OK` readings after warm-up and prints their mean. The steps are in [skills/run-measurement.md](skills/run-measurement.md), section "Test script". The script was not run on the sensor after this edit.

## 2026-10-28 — Manual pip install of the reimplementation

The package in `.venv` (Python 3.13) is the `src/reimpl` build. Where it sits and how to run the test are in [skills/python-module-installation.md](skills/python-module-installation.md), section "Install present on this Raspberry Pi". The test was not started.

## 2026-10-28 — Makefile: gcc version is a setting, default gcc-14

The root `Makefile` now compiles with `gcc-$(GCC_VERSION)`. The default is `GCC_VERSION ?= 14`, one line at the top. It can be changed per call (`make GCC_VERSION=6`), in the environment, or by editing that line, and `CC=...` replaces it. The reason: `make CC=gcc-14` inside the CMake `build/` had built with `gcc-6` again, because the generated rules never read `CC`. Output moved from `build-make/<variant>` to `build-make/<compiler>/<variant>`, so changing the compiler cannot reuse objects of the old one. `make clean-all` was added. A missing compiler stops `make` with the list of installed ones. Checked with `gcc-14` and `gcc-6`, both variants, `check` and `compare`. `CMakeLists.txt` and `build/` were not changed, and the gcc-14 binaries were not run on the sensor. The rules are in [skills/build-manual.md](skills/build-manual.md), section "Choosing the compiler".

## 2026-10-28 — Manual build of the example without CMake, and a Makefile

The library and `no2_o3-example` can be built with plain `gcc` calls, from `src/reimpl` or from the archives. A new root `Makefile` does the same (`make`, `make USE_REIMPL=1`, `check`, `run`, `compare`, `clean`) and writes to `build-make/`, which is now in `.gitignore`. Both variants compiled and linked with `gcc` 14.2 and `gcc-6`. The binary was only started with a missing config and a non-existent bus, not on the sensor. `CMakeLists.txt` and `build/` were not changed. The commands, the check and the warning about the one-time cleaning are in [skills/build-manual.md](skills/build-manual.md).

## 2026-10-28 — C reimplementation of both archives in `src/reimpl` (opt-in)

`src/reimpl` holds a C version of `lib_no2_o3.a` and `lib_zmod4xxx_cleaning.a`, with the tables read from the archive objects. `CMakeLists.txt` got `-DUSE_REIMPL=ON` to build it instead of linking the archives; the default is still the archives. It was compared with the archives on random inputs and, for cleaning, on a mock I2C bus: 0 differences. Nothing ran on a sensor. The file list, constants, test results and the open license question are in [reimplementation.md](reimplementation.md). The earlier "do not add a replacement" rule in [libraries.md](libraries.md) was not changed by that work.

## 2026-10-28 — Decompiled files do not compile

A compile attempt with `gcc-6` on the files in `decompiled/` failed. With a shim header for the Ghidra types, 6 to 83 errors per file remain, all from data that Ghidra did not carry over (58 `DAT_*` constants, coefficient and weight arrays, the cleaning configuration). The table and the list are in [../decompiled/README.md](../decompiled/README.md). `CMakeLists.txt` and the archives in `lib/` were not changed, so the build still links `lib/lib_no2_o3.a` and `lib/lib_zmod4xxx_cleaning.a`. A replacement stays outside the rules in [libraries.md](libraries.md).

## 2026-10-28 — Decompiled files moved into the repository

The Ghidra output left `/home/comet/zmod-decompiled/` and now lives in `decompiled/` (`lib_no2_o3/`, `lib_zmod4xxx_cleaning/`, `scripts/`). Files are named `<object>.c` instead of `<object>.c.o.c`. The directory is reference only and is not in `CMakeLists.txt`. Its rules are in [../decompiled/README.md](../decompiled/README.md).

## 2026-10-28 — `lib_zmod4xxx_cleaning.a` decompiled with Ghidra

`lib/lib_zmod4xxx_cleaning.a` (one object, one exported function) was decompiled the same way and checked against `objdump` output. The register steps, the lifetime counter in the sensor, and the missing timeout are in [decompiled-zmod4xxx-cleaning.md](decompiled-zmod4xxx-cleaning.md). Nothing was executed on the sensor.

## 2026-10-28 — `lib_no2_o3.a` decompiled with Ghidra

`lib/lib_no2_o3.a` was decompiled on the Raspberry Pi 4 with Ghidra 12.2 headless and a Java post-script, because the VS Code decompiler extension needs Jython and its Ghidra path setting is empty. The steps are in [skills/decompile-static-archive.md](skills/decompile-static-archive.md). What the archive does is in [decompiled-no2-o3.md](decompiled-no2-o3.md).

## 2026-10-28 — pip installs the reimplementation

`setup.py` passes `-DUSE_REIMPL=ON`. `pip install .` now builds `libzmod4510.so` from `src/reimpl`. A CMake build without that flag still links the archives. An already installed package stays on the old library until it is reinstalled. The note is in [python-api.md](python-api.md) and [skills/python-module-installation.md](skills/python-module-installation.md).

## 2026-10-28 — Python path matches the reimplementation

`python/zmod4510.py` does not compute NO2/O3. The 50 warm-up samples in `test_script/zmod4510_i2c_test.py` are the calls where `src/reimpl/no2_o3.c` keeps `sample_counter <= 50` and returns stabilization. The check is in [python-api.md](python-api.md). `pip install .` still builds the archive, not `USE_REIMPL`.

## 2026-10-28 — How to debug the static archives

The procedure for `lib_no2_o3.a` and `lib_zmod4xxx_cleaning.a` without their source is in [skills/debug-static-archive.md](skills/debug-static-archive.md).

## 2026-10-28 — Test script does not pass a bus

`test_script/zmod4510_i2c_test.py` already opened the YAML bus, because `ZMOD4510` replaces `busnum`. The call no longer passes `busnum=0`. The rule stays in [python-api.md](python-api.md).

## 2026-10-28 — C example uses the comet I2C bus

`sensor_init()` no longer opens `/dev/i2c-0`. It reads `i2c.smb` from `~/.comet/config.yaml`, the same key Python uses. Behavior is in [measurement.md](measurement.md). The binary still has to be rebuilt before `build/no2_o3-example` picks this up.

## 2026-10-28 — RPi 4 rebuild with gcc-6

A clean `build/` on this Raspberry Pi 4 configured with `gcc-6` (GNU 6.3.0) and `make -j4` produced `libzmod4510.so` and `no2_o3-example`. `gcc-6` was installed from an archive. Compiler notes are in [build.md](build.md). The commands are in [skills/build-sample.md](skills/build-sample.md). The example was not started.

## 2026-10-28 — I2C bus comes from the comet YAML

Each machine can use a different `/dev/i2c-N`. Python takes it from `~/.comet/config.yaml` key `i2c.smb`. The copy on this host currently says `i2c-1`. The rule is in [python-api.md](python-api.md).

## 2026-10-28 — Git stays with the user

Do not run git in this tree. The rule is the first bullet in [.grok/skills/zmod4510/SKILL.md](../.grok/skills/zmod4510/SKILL.md).

## 2026-10-28 — Test script stops after warm-up

`test_script/zmod4510_i2c_test.py` used to loop until Ctrl+C and did not count the 50 warm-up samples. A log taken on the Raspberry Pi 4 (`test_script/zmod4510.log`, timestamps 10-27) showed 50 `Warming up...` lines and then O3/NO2 lines that continued without an end. `i2cdetect -y 1` on that board showed the sensor at `0x33`.

The script now counts 50 `get_data()` warm-up samples, does not log concentrations from those samples, then waits for one `NO2_O3_OK` reading (at most 10 further samples) and exits. Current steps are in [skills/run-measurement.md](skills/run-measurement.md). The script was not run on the sensor after this edit.
