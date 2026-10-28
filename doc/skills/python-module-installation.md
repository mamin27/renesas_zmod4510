# Python 3 module installation

This guide documents how to install the `zmod4510` Python module from this
repository. The package is built from source and includes the native
`libzmod4510.so` library.

## Requirements

- Python 3.10 or newer
- Linux on a supported ARM or AArch64 platform
- A C/C++ toolchain and CMake
- The `ecomet_i2c_sensors` Python package. `ZMOD4510` imports
  `load_comet_yaml` from it and reads the bus number from
  `~/.comet/config.yaml` (`i2c.smb`, for example `i2c-1`). The I2C transfers
  themselves are ioctl calls inside `libzmod4510.so`, not calls into that
  package. The package is not listed in `setup.py`.

The native build is restricted to Linux ARM/AArch64 by the project's
`CMakeLists.txt`. Installing on a different operating system or architecture
will fail during the native build. Compiler and link details are in
[../build.md](../build.md).

## Install in a virtual environment

Run these commands from the repository root:

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
python -m pip install .
```

Installing with `python -m pip` uses the Python interpreter associated with
the active environment. Run the install from the repository root because
`setup.py` builds and packages the native library from the source tree.

## Verify the installation

```bash
python -c "import zmod4510; print(zmod4510.__file__)"
```

To construct the wrapper without opening the sensor:

```bash
python -c "from zmod4510 import ZMOD4510; sensor = ZMOD4510(); print(type(sensor).__name__, sensor.busnum)"
```

Construction loads `libzmod4510.so` and `~/.comet/config.yaml`. It does not
open `/dev/i2c-N`. `start()` does. The `busnum` printed here is the YAML bus,
not a default of 0, whenever that file loads. Wrapper behavior is in
[../python-api.md](../python-api.md).

## Install present on this Raspberry Pi

`python3 -m pip install .` was run by hand from `/home/comet/renesas_zmod4510`. The configure left in `_skbuild/linux-aarch64-3.13` has `USE_REIMPL=ON` and `CMAKE_C_COMPILER=/usr/bin/cc`. The link line is the objects from `src/reimpl/` plus `src/sensor_interface.c` and the HAL. `lib_no2_o3.a` and `lib_zmod4xxx_cleaning.a` are not on it.

The interpreter that received the package is `.venv/bin/python3` (Python 3.13). The library it loads is:

```text
.venv/lib/python3.13/site-packages/zmod4510/lib/libzmod4510.so
```

That file is 76432 bytes, the size recorded for a `USE_REIMPL=ON` build in [../reimplementation.md](../reimplementation.md). The archive build recorded there is 78224 bytes.

Run the test with this interpreter. A `python3` that is not the virtualenv can import an older package:

```bash
cd /home/comet/renesas_zmod4510
source .venv/bin/activate
python3 test_script/zmod4510_i2c_test.py
```

The test was not started after this install.

## Notes

- The package metadata requires Python 3.10 or newer.
- `pip install .` compiles `libzmod4510.so` from `src/reimpl`
  (`-DUSE_REIMPL=ON`) and installs it at `zmod4510/lib/libzmod4510.so`.
  The module is not pure Python.
- The native sample is a separate binary. Build it with
  [build-sample.md](build-sample.md). A CMake build without
  `-DUSE_REIMPL=ON` still links the archives.
