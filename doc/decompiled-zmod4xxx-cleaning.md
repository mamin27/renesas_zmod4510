# What `lib_zmod4xxx_cleaning.a` does (from Ghidra output)

Result of decompiling `lib/lib_zmod4xxx_cleaning.a` with Ghidra 12.2. The procedure is in [skills/decompile-static-archive.md](skills/decompile-static-archive.md). The decompiled file and the `objdump` listing are in [../decompiled/lib_zmod4xxx_cleaning/](../decompiled/lib_zmod4xxx_cleaning/) (`zmod4xxx_cleaning.c`, `zmod4xxx_cleaning.disasm.txt`). They are reference only and not built, see [../decompiled/README.md](../decompiled/README.md). The code stays proprietary Renesas software (see [libraries.md](libraries.md)). This page records structure only and is not a replacement.

The pseudocode and the AArch64 disassembly were compared and agree. Nothing was executed. Calling `zmod4xxx_cleaning_run` writes to the sensor, see "Why it is once" below.

## The archive

One object, `zmod4xxx_cleaning.c.o`: ELF64 AArch64, not stripped, `GCC: (Linaro GCC 6.3-2017.02) 6.3.1 20170109`. 2512 bytes.

| Kind | Symbols |
| --- | --- |
| Defined, global | `zmod4xxx_cleaning_run` |
| Defined, local data | `zmod4xxx_cleaning` (the configuration), `data_set_4xxx_cleaning`, `enable_write_nvm_code`, `disable_write_nvm_code`, `stop_cleaning_code` (`.bss`, one zero byte) |
| Undefined, from the open driver | `zmod4xxx_init_measurement`, `zmod4xxx_read_status`, `zmod4xxx_start_measurement` |

It uses no libm and no libc. It does its I/O only through the function pointers in the `dev` it receives (`read`, `write`, `delay_ms`). The three driver calls above are in `src/sensors/zmod4xxx.c`.

## Steps of `zmod4xxx_cleaning_run(dev)`

1. **Copy.** The 64-byte `zmod4xxx_dev_t` is copied to the stack. In the copy `meas_conf` points at the built-in `zmod4xxx_cleaning` configuration. The caller's `dev` is not changed.
2. **Check.** Read 1 byte from register `0x38` into a counter. A failed read returns `-3` (`ERROR_I2C`). A non-zero counter returns `-8` (`ERROR_CLEANING`) at once, with no cleaning and no wait.
3. **Prepare.** `zmod4xxx_init_measurement(copy)` writes the cleaning blocks. `zmod4xxx_start_measurement(copy)` writes `start` (`0xC0`) to the command register `0x93`. A non-zero result from either is returned as is.
4. **Wait for the run.** Loop: `delay_ms(50)`, `zmod4xxx_read_status`, take `status & 0x1F` (last sequencer step). Each time that value changes to `1`, a counter goes up. The value `1` is `s.len / 2 - 1` of the built-in configuration, so the last of its 2 sequencer steps. The loop ends when the counter reaches 30.
5. **Stop.** Write `0x00` (`stop_cleaning_code`) to `0x93`. Then loop `delay_ms(50)` and read status until bit `0x80` (sequencer running) is clear.
6. **Record.** `delay_ms(50)`, write `0xEA` (`enable_write_nvm_code`) to register `0xB8`. `delay_ms(50)`, write the old counter plus 1 to register `0x38`. `delay_ms(50)`, write `0x13` (`disable_write_nvm_code`) to `0xB8`.
7. **Return.** Any failed write in steps 5 and 6 returns `-3`, except the final write, whose raw I2C result is returned unchanged. After a clean pass it returns `0`.

Registers `0x38` and `0xB8` have no names in the open driver. The symbol names say the `0xB8` writes enable and disable writing to non-volatile memory. That is the library's naming, not a statement checked against the datasheet.

## Why it is once

The "run only once" rule in `zmod4xxx_cleaning.h` is not kept in the library or in the host. It is kept in the sensor. Step 6 increments register `0x38` inside the module, and step 2 refuses to run when it is not zero. This explains two points in [measurement.md](measurement.md):

- `-8` comes back immediately on a module that was cleaned before. That is why a repeated `sensor_init` is fast and only prints the skip message.
- A probe that calls `zmod4xxx_cleaning_run` on a real module changes that module for good. [skills/debug-static-archive.md](skills/debug-static-archive.md) is right to forbid it.

A failure before step 6 leaves register `0x38` at its old value, so a later run is not refused by step 2. The heater run from step 3 has already happened by then.

## Built-in configuration `zmod4xxx_cleaning`

A `zmod4xxx_conf` in `.data` (96 bytes on AArch64). Values as stored:

| Part | Register | Length | Bytes written |
| --- | --- | --- | --- |
| `start` | command `0x93` | 1 | `0xC0` |
| `h` heater | `0x40` | 2 | `fb b4` (the driver computes the real heater values from `dev->config`, see `zmod4xxx_calc_factor`) |
| `d` delay | `0x50` | 4 | `20 04 20 04` |
| `m` measurement | `0x60` | 1 | `03` |
| `s` sequencer | `0x68` | 4 | `01 c0 81 c8` |
| `r` result | `0x99` | 2 | not used by this function |
| `prod_data_len` | | | 0 |

For comparison the NO2/O3 measurement uses the same blocks at the same addresses (see the register table in [measurement.md](measurement.md)).

## Points that matter

- **No timeout.** The loop in step 4 and the loop in step 5 only end when the sensor reports the expected state, or when a status read fails. A module that never reaches step `1` thirty times, or never clears `0x80`, keeps the caller blocked. This differs from `zmod4xxx_read_sensor_info`, which gives up after 1000 polls.
- **Duration.** The status is polled every 50 ms, but the loop ends after 30 completions of the sequencer, not after 30 polls. The header says the procedure takes 1 minute, which would be about 2 s per completion. That figure is not checked against the datasheet or against a real run.
- **Errors.** `-3` covers a failed counter read and a failed write. `-8` is only the non-zero counter. Anything from `init_measurement`, `start_measurement`, or `read_status` passes through. This agrees with the way `src/sensor_interface.c` treats the result: `-8` is a skip, other non-zero values stop init.
- **No hidden API.** The only exported function is the one declared in `src/algos/zmod4xxx_cleaning.h`.
