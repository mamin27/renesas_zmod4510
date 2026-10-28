# Build the native sample

Run from the repository root. The link needs `lib/lib_no2_o3.a` and `lib/lib_zmod4xxx_cleaning.a` to be present. Do not run `lib/test_all_libs.sh` or `lib/test_all_libs2.sh` as a substitute for this build; those scripts delete the two archives. Details are in [../libraries.md](../libraries.md) and [../build.md](../build.md).

The current `build/` on this Raspberry Pi 4 was produced by the clean configure below. Reusing it keeps `gcc-6`:

```bash
cmake --build build
```

A new directory, the sequence that completed here:

```bash
rm -rf ./build
mkdir build && cd build
cmake -S .. -B . -DCMAKE_C_COMPILER=gcc-6 -DCMAKE_BUILD_TYPE=Release
make -j4
```

Same thing from the repository root, without `cd`:

```bash
cmake -S . -B build -DCMAKE_C_COMPILER=gcc-6 -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
```

The compiler is fixed when `cmake` configures a directory. `make CC=gcc-14` inside `build/` does not change it, because the generated rules name `/usr/bin/gcc-6` and never read `CC`. For another compiler configure a new directory, for example `cmake -S . -B build-gcc14 -DCMAKE_C_COMPILER=gcc-14 -DCMAKE_BUILD_TYPE=Release`. A fresh directory with `gcc-14` (GNU 14.2.0) was checked on 2026-10-28. The compiler a binary was built with is shown by `readelf -p .comment <file>`. The root `Makefile` does not have this problem: it takes the compiler from `GCC_VERSION` (default 14) or `CC`, and keeps a separate output directory per compiler. See [build-manual.md](build-manual.md), section "Choosing the compiler".

The generic `README.md` configure line omits `CMAKE_C_COMPILER`. CMake then uses the default `cc`. That has already linked this project on this machine: `_skbuild/linux-aarch64-3.11` recorded GNU 12.2.0 and `_skbuild/linux-aarch64-3.13` recorded GNU 14.2.0, and both trees contain `libzmod4510.so`. If a fresh link fails on the GCC 6.3 static archives, configure again with `gcc-6` as in the recipe above.

The binary is `build/no2_o3-example`. Its rpath points at the build directory that produced `libzmod4510.so`.

Configure fails on a non-Linux host and on a processor string that does not match `arm` or `aarch64`.
