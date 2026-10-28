# Python API

Import surface from `python/__init__.py`:

```python
from zmod4510 import ZMOD4510, ZMODStatus
```

`ZMODStatus` matches the three algorithm codes: `OK = 0`, `STABILIZATION = 1`, `DAMAGE = -102`.

## What the constructor does

`ZMOD4510(address=0x33, busnum=0, logger=None, log_level=logging.INFO, i2c=None)`:

1. Imports `ecomet_i2c_sensors.i2c` if `i2c` was not passed. The module import line also requires `load_comet_yaml` from that package. The package is not declared in `setup.py`.
2. Calls `load_comet_yaml()`. In `ecomet_i2c_sensors` that opens `~/.comet/config.yaml`. A missing file raises `FileNotFoundError` from `open`. The `-99` branch in `ZMOD4510.__init__` is only taken when that function returns `-99`, which the current `load_comet_yaml` does not do on the success path.
3. When the return value is not `-99`, replaces `busnum` with `config['i2c']['smb']` after stripping the prefix `i2c-`. The constructor argument is discarded whenever the YAML loads. The string `i2c-1` means `/dev/i2c-1`. Several machines run this tree, and each one can use a different bus. The bus in use is whatever that file says on that machine. The copy on this host currently has `i2c.smb: i2c-1`. Do not hard-code that number into a caller.
4. Loads `lib/libzmod4510.so` next to `zmod4510.py` with ctypes. Failure raises `OSError`.
5. Does not open the sensor. `start()` does that.

`ecomet_i2c_sensors` is not the I2C transport. `rpi.c` inside the shared library talks to `/dev/i2c-N`. The YAML value is only the bus number.

## Methods

| Method | C call | Notes |
| --- | --- | --- |
| `start()` | `sensor_init_with_bus(self.busnum)` | Address is always the C macro `0x33`. `self.address` is stored and never passed. `sensor_init_with_bus_and_addr` is not bound. Returns `False` when init returns non-zero. |
| `get_data(temperature_celsius_deg=-300, relative_humidity_percent=50)` | `sensor_step` | Blocks for the 6 s sample sleep inside the library. Returns a `SensorResults`. Ignores any ctypes return value. |
| `stop()` | `sensor_close` | Closes the file descriptor. |

`sensor_step` is `void` in C. The wrapper sets `restype` to `c_int` and does not read it. Status is `results.status`.

`get_data` cannot report the failures that call `HAL_HandleError`. Those exit the process. See [measurement.md](measurement.md).

`get_data` does not run the NO2/O3 math. `sensor_step` does, inside `libzmod4510.so` installed next to this file. `pip install .` passes `-DUSE_REIMPL=ON`, so that library is built from `src/reimpl`. A plain CMake build of `no2_o3-example` still links the archives unless the same flag is set. [reimplementation.md](reimplementation.md) records that `src/reimpl` matched the archive on the compared inputs. The call path is:

- Arguments are temperature, then humidity. Defaults are −300 °C and 50 % RH. −300 is outside (−280, 1 000 000), so `calc_no2_o3` uses the on-chip temperature. Humidity 50 makes `dh` zero.
- `SensorResults` is `sensor_results_t`: two floats, then three `int32`. Status `0`, `1`, and `-102` are `NO2_O3_OK`, `NO2_O3_STABILIZATION`, and `NO2_O3_DAMAGE`.
- `init_no2_o3` sets the concentrations to 0. `sample_counter` starts at 0 and increments at the beginning of each `calc_no2_o3`. While it is 50 or less the return is `NO2_O3_STABILIZATION` and the concentrations stay 0. Call 51 is the first that can be `OK` or `DAMAGE`. `DAMAGE` in those first 50 calls is not produced by `calc_no2_o3`. `sensor_step` can still set it when starting the measurement fails.

The ctypes field order must stay aligned with `sensor_results_t`. Changing one side without the other silently mis-parses the sample.

## `zmod4510_constant.py`

`ZMOD4510_ADDRESS` is `0x33`. The comment in that file says "8 bit version". The HAL programs `0x33` as a 7-bit `I2C_SLAVE` address and as the address in `i2c_msg`. An 8-bit write address would be `0x66`. Do not shift this constant.

`TEMP`, `HUMDT`, `CONF`, `HRES`, `TRES`, and the heater bits in that file are not ZMOD4510 registers and nothing in the C driver reads them. The ZMOD register map is in [measurement.md](measurement.md).

## Scripts

`python/zmod4510.py` run as `__main__` constructs `ZMOD4510()`, calls `start`, and loops on `get_data()` with the default −300 °C and 50 % RH.

`test_script/zmod4510_i2c_test.py` does not choose a bus of its own. It constructs `ZMOD4510()` and `start()` calls `sensor_init_with_bus` with the number the constructor took from `i2c.smb`. Passing `busnum=0` used to look like `/dev/i2c-0`, but the constructor discards that argument when the YAML loads, so the script was already on the file's bus. On this host that is `/dev/i2c-1`. The call no longer passes `busnum`. Changing the script does not require a rebuild of `libzmod4510.so`. After warm-up it prints 10 `OK` readings and their mean. The steps are in [skills/run-measurement.md](skills/run-measurement.md), section "Test script". It logs to `zmod4510.log` in the current directory.

`no2_o3-example` is separate. It calls `sensor_init()`, which reads the same `i2c.smb` key inside the shared library. See [measurement.md](measurement.md). That binary has to be rebuilt before it picks up a change in `sensor_init`. `sensor_init_with_bus` still uses the bus number it is given, which is how the Python `start()` method applies the YAML value.
