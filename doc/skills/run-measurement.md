# Run a measurement

Behavior of init, cleaning, the 6 s sample, and the status codes is [../measurement.md](../measurement.md). Bus selection for Python is [../python-api.md](../python-api.md).

## C example

```bash
build/no2_o3-example
```

This opens the bus named by `i2c.smb` in `~/.comet/config.yaml` at 7-bit address `0x33`. The process needs permission to open that device node. The same key is described in [../python-api.md](../python-api.md).

The first init on a module that has never been cleaned blocks for up to one minute and consumes the one lifetime cleaning. Later inits print `Skipping cleaning procedure. It has already been performed`.

The loop then blocks about 6 s per line. `Warm-Up!` is `NO2_O3_STABILIZATION`. The sample comment says to ignore concentrations until at least 50 such cycles have completed. `O3_conc` and `NO2_conc` lines are `NO2_O3_OK`. The process does not return to the shell on that path. Stop it from outside.

An unexpected algorithm status closes the device and exits non-zero. Several read failures call `exit` from `HAL_HandleError` instead of returning.

Temperature is the on-chip sensor (−300 °C sentinel). Humidity is fixed at 50 %.

## Python

Install first if `import zmod4510` fails. On this Raspberry Pi use `.venv/bin/python3`. See [python-module-installation.md](python-module-installation.md), section "Install present on this Raspberry Pi".

```bash
python3 -c "from zmod4510 import ZMOD4510; s = ZMOD4510(); print(s.start(), s.busnum); s.stop()"
```

`start()` prints the tracking number and either runs or skips cleaning, then returns. `True` means init returned 0. `s.busnum` is the bus actually opened. It comes from `~/.comet/config.yaml` key `i2c.smb` on that machine, not from the `busnum` argument. Which file owns that rule is [../python-api.md](../python-api.md).

A continuous printout of the same fields as the C example:

```bash
python3 python/zmod4510.py
```

That uses the default −300 °C and 50 % RH. `get_data(temperature_celsius_deg, relative_humidity_percent)` is the call that feeds an external reading into `calc_no2_o3`.

### Test script

`test_script/zmod4510_i2c_test.py` writes `zmod4510.log` in the working directory. On this Raspberry Pi start it with `.venv/bin/python3`. It was not run on the sensor after the edit below.

1. Fifty warm-up samples. Each line is `Warm-up N/50`. Concentrations from these calls are not logged. `sample_counter <= 50` in `src/reimpl/no2_o3.c` is this window.
2. Ten `NO2_O3_OK` samples. Each line is `1/10` through `10/10` with O3 ppb, NO2 ppb, Fast AQI, and EPA AQI.
3. One `Average` line: the arithmetic mean of those ten values, same four fields. O3 and NO2 are printed with two decimal places. The AQI means are printed with two decimal places because the mean of integers need not be an integer.

A sample after warm-up that is not `OK` is logged and the script exits with status 1. It does not print an average of a shorter set. `Ctrl+C` still closes the sensor.

`stop()` closes `/dev/i2c-N`. A read path that hits `HAL_HandleError` never reaches `stop()`; the interpreter exits.
