# Measurement

Public C entry points in `src/sensor_interface.h`:

| Function | Behavior |
| --- | --- |
| `sensor_init()` | Reads `i2c.smb` from `~/.comet/config.yaml` (`i2c-N` or a bare number) and calls `sensor_init_with_bus`. A missing file, a missing key, or a bad value returns `-1` and does not open a bus. |
| `sensor_init_with_bus(int)` | Address becomes `ZMOD4510_I2C_ADDR` (0x33, 7-bit). |
| `sensor_init_with_bus_and_addr(int, uint8_t)` | Opens that bus and 7-bit address. |
| `sensor_step(float temp, float humidity, sensor_results_t*)` | One blocking sample. Return type is `void`. |
| `sensor_close()` | Closes the I2C file descriptor. |

`sensor_results_t` field order is `o3_ppb` (float), `no2_ppb` (float), `fast_aqi` (int32), `epa_aqi` (int32), `status` (int32). The Python `SensorResults` struct uses this same order. See [python-api.md](python-api.md).

## Init

`sensor_init_with_bus_and_addr` fills `HalConfig_t`, calls `HAL_Init`, then sets:

- `dev.i2c_addr` to the address argument
- `dev.pid` to `ZMOD4510_PID` (`0x6320`)
- `dev.init_conf` and `dev.meas_conf` to the two rows of `zmod_no2_o3_sensor_cfg`
- `dev.prod_data` to a 10-byte buffer (`ZMOD4510_PROD_DATA_LEN`)

`HAL_Init` in `src/hal/raspi/rpi.c` opens `/dev/i2c-<bus>` read/write and sets `I2C_SLAVE`. Failure returns `ecHALError` (`0x100`) and does not exit. `reset` is assigned but the function is a stub that returns success. Nothing in this tree toggles a reset GPIO.

`zmod4xxx_init` checks that read, write, and `msSleep` are set, waits 200 ms, and does a zero-length I2C write as a presence probe. Failure is `ERROR_I2C` (`-3`).

`zmod4xxx_read_sensor_info` writes command 0 to the command register and polls the status register until bit 0x80 clears, or until 1000 polls of 200 ms (`ERROR_GAS_TIMEOUT`, `-2`). It reads the product id from register `0x00` (2 bytes, big-endian) and returns `ERROR_SENSOR_UNSUPPORTED` (`-4`) when it is not `dev.pid`. It then reads 6 configuration bytes from `0x20` and 10 production-data bytes from `0x26`.

The tracking number is 6 bytes at `0x3A`. Init prints it as `Sensor tracking number: x0000` plus hex, then prints the production bytes. The comment in the driver says to quote the tracking number when asking Renesas for support. The read is not required to measure.

Cleaning runs next. See below. Then `zmod4xxx_prepare_sensor`:

1. `zmod4xxx_init_sensor` writes the init heater, delay, measurement, and sequencer blocks, starts the sequencer at `0x80`, polls until the sequencer-running bit clears (50 ms poll), and reads 4 bytes at `0x97` into `mox_lr` and `mox_er`.
2. Waits 50 ms.
3. `zmod4xxx_init_measurement` writes the NO2/O3 measurement blocks the same way and does not start the sequencer.

`init_no2_o3` then initializes the algorithm handle. Its return value is the return value of `sensor_init`.

Init failures are printed as `Error during <stage> (err <n>)` and returned. They do not call `exit`.

## Cleaning

`zmod4xxx_cleaning_run` is closed source. Its register steps, read from the Ghidra output, are in [decompiled-zmod4xxx-cleaning.md](decompiled-zmod4xxx-cleaning.md). The header and the call site agree on the contract:

- It blocks for up to 1 minute.
- It is allowed once in the life of the module.
- Return 0 means it ran.
- `ERROR_CLEANING` (`-8`) means the lifetime limit is already spent. Init prints a skip message and continues.
- Any other return aborts init.

Every `sensor_init` calls it. The first successful call on a new module is the one that consumes the lifetime cleaning. Later inits only skip.

## One sample

`sensor_step`:

1. `zmod4xxx_start_measurement` writes `meas_conf->start` (`0x80`) to the command register `0x93`. On failure the function sets `out->status` to `NO2_O3_DAMAGE` and returns. It does not exit.
2. Sleeps `ZMOD4510_NO2_O3_SAMPLE_TIME` milliseconds, which is 6000. The sleep is inside the C call (`usleep`). Callers do not add their own sample delay.
3. Reads status at `0x94`. If the sequencer-running mask `0x80` is still set, it reads the error event and then calls `HAL_HandleError`. That function closes the HAL and calls `exit`. This includes the branch where the error-event read returns `ZMOD4XXX_OK`: the message is `Reading result: Wrong sensor setup!`, and `exit` still runs.
4. Reads `ZMOD4510_ADC_DATA_LEN` (32) bytes from `0x97`.
5. Reads the error event at `0xB7`. A non-zero code calls `HAL_HandleError` and exits. Bit `0x80` is `ERROR_POR_EVENT` (`-7`). Bit `0x40` is `ERROR_ACCESS_CONFLICT` (`-6`).
6. Calls `calc_no2_o3` with the raw ADC bytes, the humidity argument, and the temperature argument. The application does not call `zmod4xxx_calc_rmox`. The algorithm archive does that conversion.
7. Copies `O3_conc_ppb`, `NO2_conc_ppb`, `FAST_AQI`, and `EPA_AQI` into `sensor_results_t`, and stores the `calc_no2_o3` return code in `status`.

`HAL_HandleError` exits the process. From Python that kills the interpreter. `sensor_step` itself has no error return. A failure inside `read_and_verify` never comes back as `ZMODStatus`.

## Status values the sample understands

From `src/algos/no2_o3.h`:

| Code | Name | Meaning in `src/main.c` |
| --- | --- | --- |
| 0 | `NO2_O3_OK` | Print O3 ppb, NO2 ppb, Fast AQI, EPA AQI. |
| 1 | `NO2_O3_STABILIZATION` | Warm-up. The comment says the sensor should run at least 50 cycles and that results in this period should not be treated as valid. At 6 s per cycle that is about 5 minutes. The concentration fields may still be written. |
| −102 | `NO2_O3_DAMAGE` | Sensor self-check says the module is probably damaged. Results may be wrong. The sample keeps looping. |
| anything else | | The sample prints `Unexpected status.`, closes the sensor, and returns failure. |

`FAST_AQI` is a 1-minute EPA ozone AQI. `EPA_AQI` is the EPA ozone AQI. Both are defined that way in `no2_o3.h`. The algorithm source is the static library.

## Temperature and humidity

`src/main.c` passes −300 °C and 50 % RH on every sample. The comment there says −300 °C makes the algorithm use the gas sensor's on-chip temperature, and that an external temperature and humidity sensor is the better input. Humidity is still the constant 50 % in the sample. The Python default arguments are the same two numbers. Pass real ambient values into `sensor_step` or `ZMOD4510.get_data` when another sensor has them. This tree does not read a humidity sensor.

## Registers the open driver uses

| Address | Use |
| --- | --- |
| `0x00` | Product id, 2 bytes |
| `0x20` | Configuration, 6 bytes |
| `0x26` | Production data, 10 bytes for this product |
| `0x3A` | Tracking number, 6 bytes |
| `0x40`, `0x50`, `0x60`, `0x68` | Heater, delay, measurement, and sequencer blocks |
| `0x93` | Command / sequencer start |
| `0x94` | Status |
| `0x97` | Result block (4 bytes after init, 32 bytes after a measurement) |
| `0xB7` | Error event |

Status bits in `zmod4xxx.h`: `0x80` sequencer running, `0x40` sleep timer, `0x20` alarm, `0x1F` last sequencer step.

## Driver error codes

| Value | Name |
| --- | --- |
| 0 | `ZMOD4XXX_OK` |
| −1 | `ERROR_INIT_OUT_OF_RANGE` |
| −2 | `ERROR_GAS_TIMEOUT` |
| −3 | `ERROR_I2C` |
| −4 | `ERROR_SENSOR_UNSUPPORTED` |
| −5 | `ERROR_CONFIG_MISSING` |
| −6 | `ERROR_ACCESS_CONFLICT` |
| −7 | `ERROR_POR_EVENT` |
| −8 | `ERROR_CLEANING` |
| −9 | `ERROR_NULL_PTR` |
| `0x100` | `ecHALError` from `HAL_Init` when `/dev/i2c-N` cannot be opened or `I2C_SLAVE` fails |

I2C transactions are `I2C_RDWR`. A write of the register address followed by a read uses two messages, so the read is a repeated start. The slave address in those messages is the 7-bit address.
