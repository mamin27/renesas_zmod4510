---
name: zmod4510
description: >
  Work on the Renesas ZMOD4510 NO2/O3 driver in this repository
  (libzmod4510, no2_o3-example, the zmod4510 Python package, I2C HAL,
  cleaning, or the prebuilt algorithm archives). Use when the user builds,
  installs, runs, or changes this tree, asks how the sensor software works,
  or asks to reverse engineer, disassemble, or inspect lib_no2_o3.a or
  lib_zmod4xxx_cleaning.a, including a Raspberry Pi 4 or aarch64 build.
  Also /zmod4510.
---

# ZMOD4510

Read [doc/README.md](../../../doc/README.md) and the one file it points at for the task. Those files own the facts. Do not restate them into new notes.

When an important change lands in the same turn, append one entry to [doc/changes.md](../../../doc/changes.md) before finishing. Important means sensor behavior, the build, the linked archives, the test procedure, or anything the user asked to keep. The entry is the date, what changed, why, and the file that owns the new behavior. Do not copy that behavior into the entry. Skip a typo or a comment.

## Before changing or running anything

- Do not run git. The user handles status, diff, pull, commit, push, and gitk.
- The link uses `lib/lib_no2_o3.a` and `lib/lib_zmod4xxx_cleaning.a` only. Other trees under `lib/` are unused until copied onto those two names.
- Do not run `lib/test_all_libs.sh` or `lib/test_all_libs2.sh` unless the user asked to sweep archives. `test_all_libs2.sh` deletes those two files and wipes `build/`.
- Do not add `src/algos/zmod4510_config_no2_o3.c` to the build while `zmod4510_config_no2_o3.h` still defines the same arrays.
- `init_no2_o3` and `calc_no2_o3` have no source here. Change inputs and call sequence, not the algorithm internals.
- Leave Renesas copyright headers on the files that carry them.
- Raspberry Pi 4 64-bit archive choice and the static-analysis limit live in [doc/libraries.md](../../../doc/libraries.md) under "Raspberry Pi 4, 64-bit" and "Static analysis". Debug steps are in [doc/skills/debug-static-archive.md](../../../doc/skills/debug-static-archive.md). There is no `lib_no2_o2.a` and no `lib_zmod4xxx_clening.a`. Do not add a rewritten replacement for either proprietary archive.

## Runtime facts that are easy to get wrong

- Every `sensor_init` calls `zmod4xxx_cleaning_run`. The first success on a module blocks for up to one minute and uses the one lifetime cleaning. `ERROR_CLEANING` (-8) means it was already done.
- `sensor_step` sleeps 6000 ms inside the C call. Do not add another sample delay in Python.
- Status `1` (`NO2_O3_STABILIZATION`) is warm-up. The sample says not to treat concentrations as valid for at least 50 cycles. `test_script/zmod4510_i2c_test.py` then prints 10 `OK` readings and their mean. See [doc/skills/run-measurement.md](../../../doc/skills/run-measurement.md), section "Test script".
- `HAL_HandleError` calls `exit`. A bad read kills a Python interpreter. Init failures return an error code instead.
- The I2C bus is per machine and comes from `~/.comet/config.yaml` key `i2c.smb`, for both `ZMOD4510` and `sensor_init()` / `no2_o3-example`. Details are in [doc/python-api.md](../../../doc/python-api.md). The slave address programmed by the C API used here is 7-bit `0x33`.
- On this Raspberry Pi the installed Python package is the `.venv` one (Python 3.13, `src/reimpl`). Run tests with that interpreter. See [doc/skills/python-module-installation.md](../../../doc/skills/python-module-installation.md), section "Install present on this Raspberry Pi".
- Temperature `-300` selects the on-chip temperature. Humidity is still required. The samples pass 50 % RH.
- One sensor per process. State in `sensor_interface.c` is static.

## Where to edit

| Task | Files |
| --- | --- |
| Sample loop | `src/main.c` |
| Init, sample, cleaning call | `src/sensor_interface.c` |
| Register protocol | `src/sensors/zmod4xxx.c`, `src/sensors/zmod4xxx.h` |
| Linux I2C | `src/hal/raspi/rpi.c` |
| Product configuration blobs | `src/algos/zmod4510_config_no2_o3.h` |
| Python wrapper | `python/zmod4510.py` |
| Sensor test on the Pi | `test_script/zmod4510_i2c_test.py` |
| Build | `CMakeLists.txt`, `setup.py`, `Makefile` (manual build, see [doc/skills/build-manual.md](../../../doc/skills/build-manual.md)) |
