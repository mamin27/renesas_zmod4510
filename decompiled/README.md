# Decompiled archives (reference only)

Ghidra 12.2 pseudocode of the two prebuilt archives in `lib/`. It is for reading. It is **not part of the build**: `CMakeLists.txt` lists its sources one by one and does not include this directory, and the files do not compile (Ghidra types such as `undefined8`, unresolved `DAT_*` constants, `halt_baddata()` stubs).

| Directory | Source archive | Objects |
| --- | --- | --- |
| `lib_no2_o3/` | `lib/lib_no2_o3.a` | `no2_o3.c`, `no2_o3_NO2_nnet.c`, `no2_o3_O3_nnet.c`, `nnet_funcs.c` |
| `lib_zmod4xxx_cleaning/` | `lib/lib_zmod4xxx_cleaning.a` | `zmod4xxx_cleaning.c`, `zmod4xxx_cleaning.disasm.txt` (`objdump -dr` of the same object) |
| `scripts/` | | `DecompileAll.java`, the Ghidra post-script that produced the `.c` files |

Each `.c` file is named after the object it came from (`no2_o3.c.o` gives `no2_o3.c`). What the code does is described in [../doc/decompiled-no2-o3.md](../doc/decompiled-no2-o3.md) and [../doc/decompiled-zmod4xxx-cleaning.md](../doc/decompiled-zmod4xxx-cleaning.md). How to produce the files again is in [../doc/skills/decompile-static-archive.md](../doc/skills/decompile-static-archive.md).

## Compile attempt (2026-10-28)

`gcc-6 -std=gnu99 -fsyntax-only` on each file, run in `/tmp`, with nothing changed in this tree. Without a shim header every file fails on `undefined4`, `undefined8`, `uint`, `ulong`. With a shim header that only defines those types, errors remain:

| File | Errors left |
| --- | --- |
| `lib_no2_o3/nnet_funcs.c` | 7 |
| `lib_no2_o3/no2_o3.c` | 83 |
| `lib_no2_o3/no2_o3_NO2_nnet.c` | 6 |
| `lib_no2_o3/no2_o3_O3_nnet.c` | 6 |
| `lib_zmod4xxx_cleaning/zmod4xxx_cleaning.c` | 20 |

What is missing is data, not syntax:

- 58 distinct `DAT_<address>` constants in `no2_o3.c` and `nnet_funcs.c`. Their values are in `.rodata` of the archive objects and are not in these files.
- The arrays `p_10`, `p_20`, `p_01`, `p_02`, `p_11`, `epa_*_aqi`, `epa_*_conc`, `LONGTERMVAR_START` and the `UNK_*` offsets into them.
- The six weight arrays of each network (1200, 1260, 640, 220, 72 and 16 bytes), in `.rodata` of `no2_o3_O3_nnet.c.o` and `no2_o3_NO2_nnet.c.o`.
- `zmod4xxx_cleaning` (the configuration), `enable_write_nvm_code`, `disable_write_nvm_code`, `stop_cleaning_code`.
- Ghidra pseudo-operations (`CONCAT13`, `SUB41`, `NEON_fmov`) in `nnet_funcs_relu` and `no2_o3.c`, and `halt_baddata()` stubs for calls into other objects.

So the files cannot replace `lib/lib_no2_o3.a` or `lib/lib_zmod4xxx_cleaning.a` as they are. A working replacement would be a hand-written port with the extracted data, which [../doc/libraries.md](../doc/libraries.md) does not allow. The cleaning function would also write to the sensor's NVM, which cannot be undone.

The archives are proprietary Renesas software (see [../doc/libraries.md](../doc/libraries.md)). Do not turn these files into a replacement for them.
