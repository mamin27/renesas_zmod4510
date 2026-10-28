# Build the example by hand, without CMake

The same two targets as [build-sample.md](build-sample.md), `libzmod4510.so` and `no2_o3-example`, built with plain compiler calls. The flags are the ones CMake generated in a scratch build with `-DUSE_REIMPL=ON` (`CMakeFiles/*/flags.make` and `link.txt`). Where each source comes from is in [../architecture.md](../architecture.md), and what `USE_REIMPL` changes is in [../reimplementation.md](../reimplementation.md).

Checked on the Raspberry Pi 4 (aarch64) on 2026-10-28 with `gcc` 14.2.0 and with `gcc-6`: every command below compiled and linked, and the example started. The example was never run against the sensor, see "Run".

## With the Makefile

`Makefile` in the repository root runs the same steps. Use it unless you want to see each command.

```bash
make                    # libzmod4510.so + no2_o3-example, linked with lib/lib_*.a, compiler gcc-14
make USE_REIMPL=1       # the same, built from src/reimpl
make GCC_VERSION=6      # compile with gcc-6 for this call, see "Choosing the compiler"
make CC=/path/to/gcc    # any other compiler
make -j4 check          # build, then checks that never open an I2C bus (see step 4)
make run                # starts the example on the real sensor, read "Run" first
make compare            # src/reimpl against the archives, mock input and mock bus only
make clean              # removes the output of the current compiler
make clean-all          # removes the output of every compiler
make help               # shows which compiler would be used
```

- Output goes to `build-make/<compiler>/archive` or `build-make/<compiler>/reimpl`, for example `build-make/gcc-14/archive`. Compilers and variants do not overwrite each other. `build-make/` is in `.gitignore`. `BUILD_DIR=/some/dir` changes it. The CMake directory `build/` is not touched.
- The default is the archive variant, the same default as CMake. `USE_REIMPL` is `0` or `1`, anything else is an error.
- `CFLAGS` defaults to `-O3 -DNDEBUG`, the CMake Release flags. `-std=gnu99`, `-fPIC` and `-ffp-contract=off` for `src/reimpl` are added by the Makefile.
- A second `make` does nothing. After a header changes, only the files that include it are compiled again.
- `make run` prints the cleaning warning before it starts the binary, and for `USE_REIMPL=1` the extra note below. It does not ask for confirmation.
- `make compare` needs `lib/lib_no2_o3.a` and `lib/lib_zmod4xxx_cleaning.a`. It compiles into `/tmp` with the chosen compiler, so `build-make/` is not used.
- Do not configure CMake in the repository root (`cmake .`): an in-source CMake build writes its own `Makefile` there. Use `cmake -S . -B build`.
- With `make -j` and an archive missing (they are in `.gitignore`), the compile may start before the error stops the link. The message says to use `USE_REIMPL=1` or to put the two files into `lib/`.

Checked on the Raspberry Pi 4 on 2026-10-28: both variants with `gcc-14` (14.2.0) and with `gcc-6` (6.3.0), `make check` passed for all four, `make compare` passed with both compilers (40 000 samples and 180 cases, 0 differences), a second `make` did nothing, and `make clean` removed only its own directories. `make run` was only read with `make -n`, it was not started. `build/` kept the same checksums before and after.

## Choosing the compiler

The compiler is `gcc-$(GCC_VERSION)`. The Makefile sets it in one place, near the top:

```make
GCC_VERSION ?= 14
```

Change that number to change the default for everybody who builds with this file. For one call, or for your shell, no edit is needed:

```bash
make GCC_VERSION=6                 # one call
export GCC_VERSION=6; make         # this shell
make help                          # which compiler, where it is, its version, what is installed
```

- **Precedence:** command line, then environment, then the line in the Makefile.
- **`CC` wins.** `make CC=/usr/bin/gcc-6` or `CC=clang make` replaces `gcc-$(GCC_VERSION)`. If `GCC_VERSION` is also given on the command line, `make` prints `CC is set, so GCC_VERSION=... is ignored`.
- **Plain `gcc`:** `make GCC_VERSION=` (empty) uses the unversioned `gcc`, which is 14.2.0 on this host.
- **Checked value:** `GCC_VERSION` is pasted into a command name, so it must be one word of digits and dots (`14`, `6`). Anything else is rejected with an error before it is used in a compiler command (tried with `abc`, `14x`, `1 4` and `14; touch ...`, and no compiler ran). The check does not stop `make` itself from expanding a `$(...)` that you typed on the command line, for example `GCC_VERSION='$(shell id)'` runs `id` while the value is read. That is your own command line, but do not pass untrusted text.
- **Missing version:** `make GCC_VERSION=99` stops with the compilers found in `/usr/bin` (here `gcc-14 gcc-6`). `make help`, `make clean` and `make clean-all` still work then.
- **Own directory per compiler.** `make` compares file times only, it does not notice a different compiler. With one shared directory, switching from `gcc-14` to `gcc-6` would answer `Nothing to be done` and keep the `gcc-14` binaries. That is the mistake `make CC=gcc-14` inside the CMake `build/` made, see [build-sample.md](build-sample.md). So the output directory is `build-make/<compiler>/...`, and a `CC` with a path becomes `_usr_bin_gcc-14`. Switching compilers does not rebuild or delete the other one.
- **Which compiler is in the binary:** `make check` prints the compiler string from the binary's `.comment` section, for example `GCC: (Debian 14.2.0-19) 14.2.0`. You can read it yourself with `readelf -p .comment build-make/gcc-14/archive/no2_o3-example`. `libzmod4510.so` in the archive variant also carries `Linaro GCC 6.3` from `lib/lib_no2_o3.a`, which is the vendor's compiler and is not affected by `GCC_VERSION`.
- **`clean` and `clean-all`** remove only directories that contain the marker file `.zmod4510-make`, which the Makefile creates. A directory of the same name made by something else is left alone.

Which compiler to use: [build.md](../build.md) records that the archive was built with GCC 6.3.1 and that `gcc-6` is the matching application compiler on this Pi. Both `gcc-14` and `gcc-6` built and passed `make check` here, and both pass `make compare`. The only run on the real sensor so far (`Warm-Up!` lines) was with a `gcc-6` build from CMake. A binary from `build-make/gcc-14` was only run in the no-I2C smoke test, not on the sensor.

## Variables

The commands below are what the Makefile does, written out.

```bash
R=/home/comet/renesas_zmod4510
OUT=/tmp/zmod4510-manual          # any empty or throw-away directory
CC=gcc-14                         # or gcc-6, see "Choosing the compiler"
INC="-I$R/src -I$R/src/algos -I$R/src/sensors -I$R/src/hal -I$R/src/reimpl"
CFLAGS="-O3 -DNDEBUG -std=gnu99 -fPIC"
rm -rf "$OUT" && mkdir -p "$OUT/obj"
```

The `rm -rf` is why `OUT` must not be a directory you care about.

## 1. Driver, HAL and glue (both variants)

```bash
for f in sensors/zmod4xxx hal/zmod4xxx_hal hal/hal hal/raspi/rpi sensor_interface; do
  $CC $CFLAGS $INC -c "$R/src/$f.c" -o "$OUT/obj/$(basename $f).o"
done
```

## 2a. Algorithm and cleaning from `src/reimpl`

```bash
for f in no2_o3 no2_o3_nnet no2_o3_data no2_o3_nnet_data zmod4xxx_cleaning; do
  $CC $CFLAGS -ffp-contract=off $INC -c "$R/src/reimpl/$f.c" -o "$OUT/obj/$f.o"
done
$CC -shared -Wl,-soname,libzmod4510.so -o "$OUT/libzmod4510.so" "$OUT"/obj/*.o -lm
```

`-ffp-contract=off` is required for these five files. Without it the compiler may fuse operations the original does not fuse, and the results stop matching the archive.

## 2b. Algorithm and cleaning from the archives instead

Skip 2a. You can drop `-I$R/src/reimpl` from `INC`. This variant was checked without it.

```bash
$CC -shared -Wl,-soname,libzmod4510.so -o "$OUT/libzmod4510.so" "$OUT"/obj/*.o \
    "$R/lib/lib_no2_o3.a" "$R/lib/lib_zmod4xxx_cleaning.a" -lm
```

Pass the two `.a` files by path, after the objects. CMake uses `-l_no2_o3` with `-L$R/lib`, which is the same files.

## 3. The example

```bash
$CC -O3 -DNDEBUG -std=gnu99 -I$R/src/algos -I$R/src/sensors "$R/src/main.c" \
    -o "$OUT/no2_o3-example" -L"$OUT" -Wl,-rpath,"$OUT" -lzmod4510
```

`main.c` is not part of the library. `-Wl,-rpath,"$OUT"` bakes the absolute directory in, so the binary finds its `libzmod4510.so` without `LD_LIBRARY_PATH`. If you move the two files, either rebuild or start the binary with `LD_LIBRARY_PATH` set to the new directory.

## 4. Check before running (does not touch the bus)

`make check` does the three checks below in one go.

```bash
ldd "$OUT/no2_o3-example" | grep zmod4510    # must point into $OUT
nm -D --defined-only "$OUT/libzmod4510.so" | grep -E ' T (init_no2_o3|calc_no2_o3|zmod4xxx_cleaning_run|sensor_init)$'
```

All four names must be listed. With 2a the library needs only `libm.so.6` and `libc.so.6`. Starting the binary with a `HOME` that has no `~/.comet/config.yaml`, or one that names a bus that does not exist, ends with exit code 1 before any I2C traffic. This is a safe smoke test:

```bash
mkdir -p /tmp/home_empty && HOME=/tmp/home_empty "$OUT/no2_o3-example"; echo "exit=$?"
```

Expected: `Cannot open /tmp/home_empty/.comet/config.yaml: No such file or directory` and `exit=1`.

## Run

Read the warning first.

```bash
i2cdetect -y 1                  # bus from ~/.comet/config.yaml key i2c.smb, the sensor shows as 33
"$OUT/no2_o3-example"           # stop it with Ctrl+C
```

- The bus is `i2c.smb` from `~/.comet/config.yaml` of the user who starts the binary. On this host that is `i2c-1`. `/dev/i2c-1` is `root:i2c` with mode 660, and the user `comet` is in group `i2c`, so no `sudo` is needed. If you use it anyway, check which `HOME` the binary sees, because the config file is read from there. That depends on the sudoers setup and was not checked here.
- Every start calls `zmod4xxx_cleaning_run`. On a module that was never cleaned, the first call blocks for about a minute and uses the one cleaning in the life of the module. The cleaning writes to the sensor's non-volatile memory and cannot be undone. A module that was cleaned before prints `Skipping cleaning procedure. It has already been performed` and goes on. This note cannot tell which case your module is in.
- With step 2a that cleaning is the C version from `src/reimpl`. It was compared with the archive only on a mock I2C bus, never on a real module (see [../reimplementation.md](../reimplementation.md)). If the module may still be uncleaned and you want the vendor code to do the one irreversible write, use 2b for the first start.
- The loop prints about every 6 s. `Warm-Up!` is the first 50 samples. After that you get `O3_conc`, `NO2_conc`, `Fast AQI` and `EPA AQI` lines. The binary never exits on that path. What each status means is in [run-measurement.md](run-measurement.md) and [../measurement.md](../measurement.md).
- A failed bus read calls `exit` inside the library (`HAL_HandleError`). That is by design in this tree, see [../measurement.md](../measurement.md).
- Stop other programs that talk to the sensor first, for example a Python session. This tree has one sensor per process and does not coordinate between processes.
