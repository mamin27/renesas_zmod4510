# Debug a static archive without its source

Use this for `lib/lib_no2_o3.a` and `lib/lib_zmod4xxx_cleaning.a`. The headers `src/algos/no2_o3.h` and `src/algos/zmod4xxx_cleaning.h` are the API. The archive is a black box. Do not write a C replacement of either archive. See [../libraries.md](../libraries.md). For pseudocode instead of disassembly, see [decompile-static-archive.md](decompile-static-archive.md).

`README.md` records that `lib/lib_no2_o3.a` was built with Linaro GCC 6.3.1. On this Raspberry Pi 4 the application compiler that links it is `/usr/bin/gcc-6` (GNU 6.3.0). Check the object before running it. A crash on the first instruction is usually an ABI mismatch, not a bug in the unknown code.

## 1. Read one object

Extract outside the repository. Do not run `lib/test_all_libs.sh` or `lib/test_all_libs2.sh`.

```bash
mkdir -p /tmp/zmod-liba && cd /tmp/zmod-liba
rm -f ./*.o
ar x /home/comet/renesas_zmod4510/lib/lib_no2_o3.a
file ./*.o
readelf -h ./*.o | grep -E 'Class|Machine'
readelf -p .comment ./*.o
readelf -A ./*.o
nm -g --defined-only ./*.o
```

On this Pi the object must be `ELF64` and `AArch64`. `.comment` should name GCC 6.3. `ELF32` or `Machine: ARM` means a 32-bit archive. No compiler on this 64-bit system will link it.

`readelf -A` shows soft-float versus hard-float only for 32-bit ARM (`Tag_ABI_VFP_args`). AArch64 has no soft-float ABI. A mismatch there is the class or the machine, not a VFP tag.

Repeat the same commands for `lib/lib_zmod4xxx_cleaning.a`.

## 2. Match symbols to the headers

The defined globals that matter are `init_no2_o3`, `calc_no2_o3`, and `zmod4xxx_cleaning_run`. Their prototypes are in the two headers above. `lib/test.c` and `lib/test_all_libs.sh` declare different prototypes. A crash from those files says nothing about the archive.

If `nm` prints no names, the object is stripped. The only next step is `disassemble` at the address from the crash.

## 3. Debug the binary that already links them

`build/libzmod4510.so` is the black box with symbols, built by `gcc-6 -shared -fPIC` and `-l_no2_o3 -l_zmod4xxx_cleaning`. `build/no2_o3-example` loads that `.so`.

```bash
gdb --args /home/comet/renesas_zmod4510/build/no2_o3-example
```

```
break init_no2_o3
break calc_no2_o3
run
disassemble
info registers
bt
x/16i $pc
```

There is no C source for those functions. `disassemble` is the view. `ltrace` does not see them: after the static link they are direct calls, not PLT slots in a separate `.so`.

Do not call `zmod4xxx_cleaning_run` from a throwaway probe. A successful run can consume the one lifetime cleaning of the sensor. `sensor_init` already calls it.

The example reads `i2c.smb` from `~/.comet/config.yaml` and then loops. Stop it from gdb. Do not leave it running.

## 4. Link errors, before execution

```bash
gcc-6 -g -O0 -fno-omit-frame-pointer -o /tmp/probe /tmp/probe.c \
  /home/comet/renesas_zmod4510/lib/lib_no2_o3.a -Wl,-Map=/tmp/probe.map
nm -u /tmp/probe | head
```

`__aeabi_*` in that list means the object is 32-bit ARM. Those names do not exist in an AArch64 archive. `__stack_chk_fail` means the object was built with stack protection and the link needs the normal libgcc and libc. It is not an algorithm fault.

An executable probe can link a non-PIC object. `libzmod4510.so` cannot. The failure looks like `relocation R_AARCH64_... can not be used when making a shared object`. That is a PIC mismatch. The current `build/libzmod4510.so` already linked, so the archives in `lib/` passed that check with `gcc-6`.
