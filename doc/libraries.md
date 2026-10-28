# Prebuilt libraries

The application link uses only these two files:

- `lib/lib_no2_o3.a` — `init_no2_o3`, `calc_no2_o3`
- `lib/lib_zmod4xxx_cleaning.a` — `zmod4xxx_cleaning_run`

`README.md` records that `lib/lib_no2_o3.a` was built with Linaro GCC 6.3.1 20170109. Replacing either file changes the algorithm or the cleaning routine for every later build. CMake does not select a subdirectory by CPU.

Names that do not exist in this tree: `lib_no2_o2.a`, `lib_zmod4xxx_clening.a`. The product is NO2 and O3, and the cleaning archive uses the spelling `cleaning`.

## Raspberry Pi 4, 64-bit

Raspberry Pi 4 running a 64-bit OS is AArch64 Linux (`aarch64-linux-gnu`), not 32-bit `arm-linux-gnueabihf` and not bare-metal `arm-none-eabi`.

| Role | Path |
| --- | --- |
| Pair the current CMake link uses | `lib/lib_no2_o3.a`, `lib/lib_zmod4xxx_cleaning.a` |
| Catalog copy labeled for this target | `lib/Raspberry Pi/ARMv8-A x64/aarch64-linux-gnu-gcc/` (same two filenames) |

The root pair has already been linked into `libzmod4510.so` under `_skbuild/linux-aarch64-3.11` and `_skbuild/linux-aarch64-3.13`. Those shared objects are AArch64, so the archives that were on the link line are AArch64 Linux objects. The ELF class was not dumped again after that; the conclusion is from the successful link.

The folder `lib/Raspberry Pi/ARMv8-A x86/` is 32-bit hard-float despite the `x86` in the name. Cortex-A `arm-none-eabi` copies are not Raspberry Pi OS objects. A second AArch64 Linux pair sits in `lib/Arm Cortex-A/A35/aarch64-linux-gnu-gcc/`.

Byte identity of the root pair with `ARMv8-A x64` or with the A35 pair has not been compared. Compare before replacing either root archive.

## Static analysis

The public entry points are already declared in `src/algos/no2_o3.h` and `src/algos/zmod4xxx_cleaning.h`: `init_no2_o3`, `calc_no2_o3`, `zmod4xxx_cleaning_run`. Disassembly can show how those three are implemented (symbols, calls, the handle fields, the cleaning return `-8`). It does not uncover another measurement API.

The same headers state that the implementation is proprietary Renesas gas-sensor software. Static analysis of the AArch64 archives above is in scope when the user asks for it. A rewritten C replacement of either archive is not. Do not add such a replacement to the tree. The commands are in [skills/debug-static-archive.md](skills/debug-static-archive.md). Decompiling with Ghidra is in [skills/decompile-static-archive.md](skills/decompile-static-archive.md), and the results are in [decompiled-no2-o3.md](decompiled-no2-o3.md) for `lib_no2_o3.a` and [decompiled-zmod4xxx-cleaning.md](decompiled-zmod4xxx-cleaning.md) for `lib_zmod4xxx_cleaning.a`.

A C version of both archives now exists in `src/reimpl` and is built only with `-DUSE_REIMPL=ON`. The paragraph above was not edited when it was added, so the two disagree. What the reimplementation is, how it was checked and the license question are in [reimplementation.md](reimplementation.md).

## Catalog

`lib/` also stores the same pair of archives built for other cores and toolchains. Those copies are unused until something copies them onto the two filenames above.

| Directory | Toolchains present |
| --- | --- |
| `lib/Raspberry Pi/ARMv6`, `ARMv7-A`, `ARMv8-A x86` | `arm-linux-gnueabihf-gcc` |
| `lib/Raspberry Pi/ARMv8-A x64` | `aarch64-linux-gnu-gcc` |
| `lib/Arm Cortex-A/A35` | `aarch64-linux-gnu-gcc` |
| `lib/Arm Cortex-A/A7`, `A9`, `A15`, `A53`, `A57` | `arm-none-eabi-gcc`; A7, A9, and A15 also have `iar-ew-arm` |
| `lib/Arm Cortex-M/M0`, `M0plus`, `M1`, `M3`, `M4`, `M4f`, `M7`, `M7f` | `arm-none-eabi-gcc`, `armcc`, `armclang`, `iar-ew-arm`, `iar-ew-synergy-arm` where the tree has them |
| `lib/Arm Cortex-M/M23`, `M33` | no `armcc` pair |
| `lib/Arm Cortex-R/R4` | `arm-none-eabi-gcc`, `iar-ew-arm` |
| `lib/Arm Linux/armv5e` | `arm-linux-gcc` |
| `lib/Linux/x64`, `lib/Linux/x86` | `gcc` |

Bare-metal archives (`arm-none-eabi-gcc`, `armcc`, `armclang`, IAR) are the wrong objects for this Linux shared library. The host builds recorded under `_skbuild/linux-aarch64-*` are AArch64 Linux. The matching catalog pairs are `lib/Raspberry Pi/ARMv8-A x64/aarch64-linux-gnu-gcc/` and `lib/Arm Cortex-A/A35/aarch64-linux-gnu-gcc/`. Whether the files currently in `lib/*.a` are byte-identical to one of those pairs is not recorded here; compare them before replacing.

This CMake project still refuses to configure on x86_64, so the `lib/Linux/x64` and `x86` archives do not build `libzmod4510.so` on this tree without changing `CMakeLists.txt`.

## Scripts under `lib/`

`lib/test_all_libs2.sh` must be started with `lib/` as the working directory. For every nested `lib_no2_o3.a` it:

1. Deletes `lib/lib_no2_o3.a` and `lib/lib_zmod4xxx_cleaning.a`.
2. Copies that candidate pair onto those two names.
3. Deletes the contents of `build/`.
4. Configures with `gcc-6` and runs `make -j4`.
5. Runs `build/no2_o3-example` for at most 3 seconds.
6. Deletes the two archives again.

After a finished run the two archives the normal build links are gone. A successful trial is either a timeout or a clean exit, or stderr that contains both `Failed to write to the I2C device` and `Error during sensor initialization`. The second case means the binary ran far enough to talk I2C and the sensor did not answer. It does not mean a concentration was computed.

`lib/test_all_libs.sh` is an older probe. It also copies candidate archives onto `lib/*.a`. The generated `test.c` declares `init_no2_o3` and `calc_no2_o3` with prototypes that do not match `src/algos/no2_o3.h`, then calls them. A crash or a successful link from that file says nothing reliable about the algorithm API.

`lib/test.c` is the same mismatched prototype file, left on disk.

Do not run either script as a build check. Building the sample is [skills/build-sample.md](skills/build-sample.md).
