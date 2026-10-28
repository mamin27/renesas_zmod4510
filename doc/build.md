# Build

`CMakeLists.txt` stops the configure step unless `CMAKE_SYSTEM_NAME` is Linux and `CMAKE_SYSTEM_PROCESSOR` matches `arm` or `aarch64`. C standard is C99. The shared library is linked with libm.

## Targets

| Target | Contents |
| --- | --- |
| `libzmod4510.so` | `src/sensors/zmod4xxx.c`, `src/hal/zmod4xxx_hal.c`, `src/hal/hal.c`, `src/hal/raspi/rpi.c`, `src/sensor_interface.c`, plus `lib/lib_no2_o3.a` and `lib/lib_zmod4xxx_cleaning.a` |
| `no2_o3-example` | `src/main.c` linked to `libzmod4510.so` |

The link line uses `-l_no2_o3` and `-l_zmod4xxx_cleaning` with `-L<repo>/lib`. The linker therefore opens `lib/lib_no2_o3.a` and `lib/lib_zmod4xxx_cleaning.a`. Archives under `lib/Raspberry Pi/`, `lib/Arm Cortex-*/`, and the rest are not on that link line. See [libraries.md](libraries.md).

`cmake -DUSE_REIMPL=ON` drops those two archives from the link line and compiles `src/reimpl/*.c` into `libzmod4510.so` instead (with `-ffp-contract=off`). The default is OFF. See [reimplementation.md](reimplementation.md).

The root `Makefile` is a second way to build the same two targets, without CMake and into `build-make/<compiler>/`. Its compiler is `gcc-$(GCC_VERSION)`, currently `gcc-14`, set on one line at the top and changeable per call (`make GCC_VERSION=6`). See [skills/build-manual.md](skills/build-manual.md), section "Choosing the compiler".

`no2_o3-example` is linked with `-Wl,-rpath,<build-dir>` and `libzmod4510.so`, so the binary from `cmake --build build` loads `build/libzmod4510.so` without `LD_LIBRARY_PATH`.

The only `install()` rule places the shared library at `python/lib` inside the scikit-build install tree. A plain `cmake --install` is not how the sample is meant to be run. The Python package picks the library up from next to `zmod4510.py`. See [skills/python-module-installation.md](skills/python-module-installation.md).

Commands that produce the sample are in [skills/build-sample.md](skills/build-sample.md).

## Compilers already used in this tree

The current `build/` on this Raspberry Pi 4 was configured from a clean directory with `-DCMAKE_C_COMPILER=gcc-6 -DCMAKE_BUILD_TYPE=Release`. CMake identified `/usr/bin/gcc-6` as GNU 6.3.0. `make -j4` then built both targets. The shared library and `no2_o3-example` are linked with `/usr/bin/gcc-6`. C++ was not requested; CMake still probed `/usr/bin/c++`, which is GNU 14.2.0, and did not use it for these targets.

Configure prints a deprecation warning because `CMakeLists.txt` has `cmake_minimum_required(VERSION 3.0)`. The build finishes anyway.

Older configure results still on disk:

| Tree | `CMAKE_C_COMPILER` | Identified version |
| --- | --- | --- |
| `_skbuild/linux-aarch64-3.11/` | `/usr/bin/cc` | GNU 12.2.0 |
| `_skbuild/linux-aarch64-3.13/` | `/usr/bin/cc` | GNU 14.2.0 |

Those trees also contain a `libzmod4510.so`, so GCC 12 and 14 have linked this project on linux-aarch64 against the archives in `lib/` as well.

`README.md` records `strings` output on `lib/lib_no2_o3.a`: Linaro GCC 6.3.1 20170109. That is the compiler that built the archive. On this Pi the matching application compiler is `gcc-6` (GNU 6.3.0), installed from an archive because the link needs it. Commands are in [skills/build-sample.md](skills/build-sample.md).

## Python package build

`setup.py` calls scikit-build. `pyproject.toml` declares `setuptools`, `wheel`, and `scikit-build`, with backend `setuptools.build_meta`. The package name is `zmod4510`, version 0.2.0, `python_requires >= 3.10`. Sources live in `python/` and are mapped onto the import package `zmod4510`.

scikit-build runs this same `CMakeLists.txt` with `-DUSE_REIMPL=ON` from `setup.py`. The `.so` that `import zmod4510` loads is therefore the `src/reimpl` build. The install prefix used by the recorded builds places:

```
zmod4510/__init__.py
zmod4510/zmod4510.py
zmod4510/zmod4510_constant.py
zmod4510/lib/libzmod4510.so
```

`ZMOD4510.__init__` loads `lib/libzmod4510.so` relative to `zmod4510.py`. A `libzmod4510.so` that exists only in `build/` is not the file the Python module opens after `pip install`.

`README.md` shows an older installed version 0.1.0 from a previous `pip list`. `setup.py` now says 0.2.0.
