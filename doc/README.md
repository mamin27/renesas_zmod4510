# ZMOD4510 software notes

Notes on the tree in this repository. Vendor PDFs in this directory are not transcribed.

| File | What it owns |
| --- | --- |
| [architecture.md](architecture.md) | Layers, source files, call path, licenses as written in the tree |
| [build.md](build.md) | CMake targets, compilers that have already produced `libzmod4510.so`, package layout |
| [libraries.md](libraries.md) | Which static archives the link uses, the Raspberry Pi 4 AArch64 pair, static-analysis limits, and what `lib/test_all_libs*.sh` does |
| [reimplementation.md](reimplementation.md) | `src/reimpl`, the opt-in C version of both archives (`USE_REIMPL`): files, constants read from them, how it was compared with the archives, open license question |
| [measurement.md](measurement.md) | Init, cleaning, one sample, status codes, I2C registers, process-exit paths |
| [python-api.md](python-api.md) | `zmod4510.ZMOD4510`, ctypes layout, bus and address selection |
| [skills/python-module-installation.md](skills/python-module-installation.md) | `pip install .` |
| [skills/build-sample.md](skills/build-sample.md) | Configure and build `no2_o3-example` |
| [skills/build-manual.md](skills/build-manual.md) | Build `libzmod4510.so` and `no2_o3-example` with the root `Makefile` or plain `gcc` calls, with `src/reimpl` or with the archives, then run it |
| [skills/debug-static-archive.md](skills/debug-static-archive.md) | Check and debug `lib_no2_o3.a` and `lib_zmod4xxx_cleaning.a` without their source |
| [skills/decompile-static-archive.md](skills/decompile-static-archive.md) | Decompile `lib_no2_o3.a` with Ghidra headless on the Pi |
| [decompiled-no2-o3.md](decompiled-no2-o3.md) | What `lib_no2_o3.a` does, read from the Ghidra output: data flow, neural network shape, status rules |
| [decompiled-zmod4xxx-cleaning.md](decompiled-zmod4xxx-cleaning.md) | What `lib_zmod4xxx_cleaning.a` does, read from the Ghidra output: register steps, why it runs once, error returns |
| [skills/run-measurement.md](skills/run-measurement.md) | Run the example or the Python loop and read the status |
| [changes.md](changes.md) | Log of important changes. Behavior stays in the topic file each entry names |

Ghidra pseudocode of the two archives is in `../decompiled/`. It is reference only and not built. See [../decompiled/README.md](../decompiled/README.md).

Vendor files already here:

- `REN_ZMOD4510_DST_20250327.pdf` — Renesas datasheet (DST), dated 2025-03-27
- `REN_ZMOD4510_MAT_20240429_1.pdf` — Renesas companion manual (MAT), dated 2024-04-29

The C comments point at the datasheet for conditioning and self-check, and at the programming manual for error codes. Those PDFs are the copies stored with this tree.
