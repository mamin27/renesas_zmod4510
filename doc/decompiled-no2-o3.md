# What `lib_no2_o3.a` does (from Ghidra output)

Result of decompiling `lib/lib_no2_o3.a` with Ghidra 12.2. The procedure is in [skills/decompile-static-archive.md](skills/decompile-static-archive.md). The decompiled files are in [../decompiled/lib_no2_o3/](../decompiled/lib_no2_o3/) (reference only, not built, see [../decompiled/README.md](../decompiled/README.md)). The code stays proprietary Renesas software (see [libraries.md](libraries.md)). This page records structure and data flow only. It is not a replacement and none should be added.

Everything below is read from pseudocode. Threshold and filter constants are `DAT_*` values in `.rodata`, and this page gives no numbers for them. They were read from the objects later for `src/reimpl` and are listed in [reimplementation.md](reimplementation.md). Statements marked "inferred" come from field use and the public headers, not from a vendor description.

## Objects in the archive

All are ELF64 AArch64, built with Linaro GCC 6.3.1, with symbols.

| Object | Global symbols | Role |
| --- | --- | --- |
| `no2_o3.c.o` | `init_no2_o3`, `calc_no2_o3`, `init_no2_o3_internal`, `calc_no2_o3_internal`, `no2_o3_ver` | Public API, ADC to resistance, filtering, AQI |
| `no2_o3_NO2_nnet.c.o` | `calc_no2_o3_NO2_nnet` | NO2 neural network |
| `no2_o3_O3_nnet.c.o` | `calc_no2_o3_O3_nnet` | O3 neural network |
| `nnet_funcs.c.o` | `nnet_funcs_dotbias`, `nnet_funcs_tanh`, `nnet_funcs_relu`, `nnet_funcs_safe_exp` | Layer helpers |

Undefined in the archive and taken from libm at link time: `exp`, `log10`, `pow`. That is why the link needs `-lm`.

`init_no2_o3` and `init_no2_o3_internal` have identical bodies. `calc_no2_o3` is the wrapper and `calc_no2_o3_internal` does the work.

## `init_no2_o3`

Writes into `no2_o3_handle_t`. Every field is 4 bytes, 18 words in total:

| Words | Field in `no2_o3.h` | After init |
| --- | --- | --- |
| 0 | `sample_counter` | 0 |
| 1–4 | `rmoxs_smooth[4]` | 0 |
| 5–8 | `logrmoxs_mean[4]` | 0 |
| 9–12 | `logrmoxs_var[4]` | start values from two 8-byte constants (`LONGTERMVAR_START` and a second one) |
| 13–17 | `o3_1min_ppb`, `o3_1h_ppb`, `o3_8h_ppb`, `no2_1min_ppb`, `no2_1h_ppb` | 0 |

It always returns 0.

## `calc_no2_o3`: ADC to resistance

`calc_no2_o3(handle, dev, inputs, results)` reads the 32 ADC bytes from `inputs->adc_result` as big-endian 16-bit words. Four of them are the MOx channels: words 4, 9, 11 and 15 (bytes 8, 18, 22 and 30). For each one, with `lr = dev->mox_lr` and `er = dev->mox_er`:

- adc ≤ lr: the result is the lower clamp.
- lr < adc < er: `rmox = dev->config[0] * K * (adc - lr) / (er - adc)`, clamped between a lower and an upper constant.
- adc ≥ er: the result is the upper clamp.

The four values go to `results->rmox[0..3]`. This is the `zmod4xxx_calc_rmox` job. It is done inside the archive, which matches [measurement.md](measurement.md).

Offsets used on AArch64: `config[6]` at byte 1, `mox_er` at 8, `mox_lr` at 10, `prod_data` at 16 in `zmod4xxx_dev_t`.

It then calls `calc_no2_o3_internal` with:

1. Word 0 of the ADC data (bytes 0–1) as `C0 + C1 * word`. Inferred: the raw on-chip temperature term.
2. `config[2..3]` (big-endian) divided by a constant.
3. `config[5] * 0.5`.
4. `inputs->humidity_pct`.
5. `inputs->temperature_degc`.
6. The handle.
7. `results` (its first member is `rmox[4]`, so the same pointer is also the resistance array).
8. `dev->prod_data[9]`.

Ghidra shows only 8 arguments in the call. The function body uses a 9th, the `results` pointer again, at offsets `+0x10` (temperature), `+0x14` (O3), `+0x18` (NO2), `+0x1c` (FAST_AQI) and `+0x1e` (EPA_AQI). Those offsets match `no2_o3_results_t` in `no2_o3.h`, so the missing argument is a decompiler artefact.

## `calc_no2_o3_internal`: per-sample flow

1. **Counter.** `sample_counter` is incremented unless it already equals `0xFFFFFFFF`. That is the saturation the header names.
2. **Temperature.** If `temperature_degc` is outside a lower and an upper constant, the on-chip temperature is used, computed as `((p1 / p2 - 1) / C + p3) - C2`. Otherwise `temperature_degc` is used. The result is stored at `results->temperature`. This matches the −300 °C remark in [measurement.md](measurement.md): −300 is below the lower constant.
3. **Smoothing of resistance.**
   - Samples 1 to 50: `rmoxs_smooth[i] = rmox[i]`.
   - Later: `rmoxs_smooth[i] *= pow(rmox[i] / rmoxs_smooth[i], C)`.
4. **Log and compensation.** `log10(rmoxs_smooth[i])` is multiplied by `1 + p10*dT + p20*dT^2 + p01*dH + p02*dH^2 + p11*dT*dH`, with `dT = T - 20` and `dH = humidity - C`. The coefficient sets `p_10`, `p_20`, `p_01`, `p_02`, `p_11` hold 4 floats each, one per channel.
5. **Long-term mean and variance.**
   - Samples 1 to 50: the compensated logs are stored as the mean. The variance keeps its start value.
   - Later: a running mean and variance with `alpha = 1 - exp(x)`. `x` is `-1 / counter`, clamped to a range, and a fixed constant once the counter is past 0x23281 (144 001). The variances are floored with one constant per channel.
6. **Neural networks** (counter above 50 only). The input is 14 floats:

   | Index | Content |
   | --- | --- |
   | 0 | `humidity_pct` |
   | 1 | the temperature from step 2 |
   | 2 to 13 | for each channel i, three values in this order: compensated log, long-term mean, long-term variance |

   `calc_no2_o3_O3_nnet` fills the O3 estimate and `calc_no2_o3_NO2_nnet` fills the NO2 estimate.
7. **Scale.** Both estimates are multiplied by `pow(C, prod_data[9])`. If `prod_data[9]` is 128 or more, a constant (most likely 256) is subtracted from the exponent first, so it behaves as a signed byte. Inferred: a per-sensor calibration from the production data. Negative results are set to 0.
8. **Concentration filters.** Exponential averages with three different constants. O3 has three outputs: 1 min, 1 h and 8 h. NO2 has two: 1 min and 1 h. They are the five fields at the end of the handle.
9. **Outputs.**
   - `results->O3_conc_ppb` is `o3_1min_ppb`. `results->NO2_conc_ppb` is `no2_1min_ppb`.
   - Each concentration goes through a piecewise-linear table (`epa_o3_1h_aqi`, `epa_o3_8h_aqi`, `epa_no2_1h_aqi` and their breakpoint constants). Below the first breakpoint the AQI is a straight multiple. Above the last breakpoint it is a fixed maximum.
   - `FAST_AQI` is the larger of the 1-minute O3 value (through the 1 h O3 table) and the 1-minute NO2 value (through the 1 h NO2 table).
   - `EPA_AQI` is the largest of the O3 1 h, O3 8 h and NO2 1 h AQI values.
10. **Return value.**
    - Counter 50 or lower: `1`, `NO2_O3_STABILIZATION`.
    - Otherwise `-102` (`NO2_O3_DAMAGE`, seen as `0xffffff9a`) if `rmox[3]` is outside a lower and an upper constant, or if the first value passed to the function (step 1 of the argument list) is above a limit.
    - Otherwise `0`, `NO2_O3_OK`.

The status becomes `NO2_O3_OK` at the 51st sample, not the 50th. The warm-up rule in `no2_o3.h` and [measurement.md](measurement.md) (at least 50 cycles) agrees with that.

## Neural networks

Both networks have the same shape and call the same helpers. Only the weight arrays differ: `no2_o3_O3_nnet_layer{0,2,4,6,8,10}_bias_weights` and the NO2 equivalents.

| Layer | Inputs | Outputs | Activation |
| --- | --- | --- | --- |
| 0 | 14 | 20 | tanh |
| 2 | 20 | 15 | tanh |
| 4 | 15 | 10 | tanh |
| 6 | 10 | 5 | tanh |
| 8 | 5 | 3 | tanh |
| 10 | 3 | 1 | none, written straight to the output |

Each layer is `nnet_funcs_dotbias(in, out, n_in, n_out, weights)` followed by `nnet_funcs_tanh(out, out, n_out)`. Two stack buffers (80 and 64 bytes) are reused alternately.

`nnet_funcs` helpers:

- `nnet_funcs_dotbias`: for each output neuron, start from a bias, then add `in[j] * w`. The weights are laid out per neuron as one bias followed by `n_in` weights.
- `nnet_funcs_tanh`: `(e - 1/e) / (e + 1/e)` with `e = exp(x)`. It returns exactly +1 or −1 outside a pair of limits.
- `nnet_funcs_relu`: `max(x, 0)`. Neither network calls it.
- `nnet_funcs_safe_exp`: `exp` with a clamp to a pair of limits. Neither network calls it.

## What this gives you

- The API in `no2_o3.h` is the only public surface. There is no hidden measurement entry point.
- A temperature argument below the lower limit selects the on-chip temperature, and the algorithm reads `prod_data[9]`. `dev.prod_data` must therefore hold the 10 bytes the init sequence read, not zeroes.
- Humidity is a direct network input as well as part of the compensation. The constant 50 % in `src/main.c` changes the result.
- The values behind every threshold, filter constant, coefficient set, and weight array are in the object files. They were extracted later: the tables are in `src/reimpl/no2_o3_data.c` and `src/reimpl/no2_o3_nnet_data.c`, the scalar constants are in [reimplementation.md](reimplementation.md).
