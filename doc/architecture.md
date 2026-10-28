# Architecture

Outdoor NO2 and O3 firmware for the Renesas ZMOD4510, trimmed for a Linux I2C bus on ARM or AArch64. `README.md` records the local changes against the Renesas sample: pigpio is gone, the on-board temperature and humidity sensor path is gone, the algorithm archives shipped for the link are the ones in `lib/`, and the build is CMake.

Package version in `setup.py` is 0.2.0. Header versions are HAL and ZMOD API 2.7.1, NO2/O3 algorithm 1.0.1.

## Layers

```
src/main.c                         python/zmod4510.py
    sensor_init / sensor_step          ctypes calls of the same symbols
              \                        /
               libzmod4510.so
                    sensor_interface.c     one process-global sensor
                    zmod4xxx.c             register protocol
                    zmod4xxx_hal.c         maps read/write/delay onto Interface_t
                    hal.c                  error string storage
                    hal/raspi/rpi.c        /dev/i2c-N via ioctl I2C_RDWR
                    + lib/lib_no2_o3.a             init_no2_o3, calc_no2_o3
                    + lib/lib_zmod4xxx_cleaning.a  zmod4xxx_cleaning_run
```

`no2_o3-example` is `src/main.c` linked to `libzmod4510.so`. The shared library does not include `main.c`.

There is one sensor per process. `sensor_interface.c` keeps the HAL, the device object, the ADC buffer, the production-data buffer, and the algorithm handle in static storage. A second `sensor_init` reuses that state.

## Source map

| Path | Role |
| --- | --- |
| `src/main.c` | Sample loop. Bus 0, address 0x33, temperature −300 °C, humidity 50 %. Never returns on the success path. |
| `src/sensor_interface.h` | Public C API and `sensor_results_t`. |
| `src/sensor_interface.c` | Init, one measurement, close. |
| `src/sensors/zmod4xxx.h` | Register addresses, status masks, driver API. |
| `src/sensors/zmod4xxx.c` | Driver. Open source. |
| `src/sensors/zmod4xxx_types.h` | `zmod4xxx_dev_t`, `zmod4xxx_conf`, driver error enum. |
| `src/hal/hal.h`, `src/hal/hal.c` | `Interface_t`, `HalConfig_t`, last-error storage. |
| `src/hal/zmod4xxx_hal.c` | Binds the legacy driver function pointers to `Interface_t`. |
| `src/hal/raspi/rpi.c` | Linux I2C HAL. The name is historical; it does not use pigpio. |
| `src/algos/no2_o3.h` | Algorithm types and return codes. Implementation is in `lib_no2_o3.a`. |
| `src/algos/zmod4xxx_cleaning.h` | `zmod4xxx_cleaning_run`. Implementation is in `lib_zmod4xxx_cleaning.a`. |
| `src/algos/zmod4510_config_no2_o3.h` | Product id, bus, address, sample time, and the init/measurement blobs. The arrays are defined in this header. |
| `src/algos/zmod4510_config_no2_o3.c` | Same arrays again. CMake does not compile this file. |
| `python/zmod4510.py` | ctypes wrapper. |
| `python/zmod4510_constant.py` | Imported for `ZMOD4510_ADDRESS` only. The other names are a humidity-sensor register map and are unused. |
| `python/__init__.py` | Exports `ZMOD4510` and `ZMODStatus`. |
| `setup.py`, `pyproject.toml`, `CMakeLists.txt` | Package and native build. |
| `lib/` | Prebuilt algorithm and cleaning archives. See [libraries.md](libraries.md). |
| `test_script/zmod4510_i2c_test.py` | Python loop that logs to `zmod4510.log` in the current directory. |

`sensor_interface.c` includes `zmod4510_config_no2_o3.h`, so the live configuration symbols (`data_set_4510_init`, `data_set_4510_no2_o3`, `zmod_no2_o3_sensor_cfg`) come from that header. Adding the `.c` file to `CMakeLists.txt` without removing those definitions from the header produces duplicate symbols.

## Licenses as they appear in the tree

The repository root `LICENSE` is GPL-3. The Renesas HAL, driver, algorithm, and cleaning headers each say they are proprietary and point at the Renesas gas-sensor software license terms. The algorithm and cleaning implementations are binary archives, not source. Leave those copyright headers and the archives in place when editing.
