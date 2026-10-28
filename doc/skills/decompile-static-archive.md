# Decompile a static archive with Ghidra

Use this to produce C-like pseudocode from `lib/lib_no2_o3.a` (or `lib/lib_zmod4xxx_cleaning.a`) for reading. It is analysis only. Do not add the output or a rewrite of the archive to this tree. See [../libraries.md](../libraries.md). What came out is in [../decompiled-no2-o3.md](../decompiled-no2-o3.md) for `lib_no2_o3.a` and in [../decompiled-zmod4xxx-cleaning.md](../decompiled-zmod4xxx-cleaning.md) for `lib_zmod4xxx_cleaning.a`.

## What is installed on the Pi

| Item | Where |
| --- | --- |
| VS Code extension `tintinweb.vscode-decompiler-0.1.0` | `~/.vscode-server/extensions/` |
| Ghidra 12.2 | `/home/comet/ghidra_12.2_PUBLIC` |
| Java | OpenJDK 25 |

The extension is a wrapper. It runs `<ghidra>/support/analyzeHeadless`, imports the file into a temporary project, and runs `scripts/ghidra_decompile.py` as a post-script. It reads the setting `vscode-decompiler.tool.ghidra.path`. On this host that setting is empty and `ghidraRun` is not in `PATH`, so the extension cannot find Ghidra by itself. Set the setting to:

```
/home/comet/ghidra_12.2_PUBLIC/support/analyzeHeadless
```

`ghidra_decompile.py` is a Jython script. This Ghidra has no Jython installed: there is no `Ghidra/Features/Jython`, and `Extensions/Ghidra/ghidra_12.2_PUBLIC_20261016_Jython.zip` is not unpacked. The extension route was not run here. The headless run below was used instead, with a Java post-script that does the same job.

## 1. Extract the objects

An archive is a set of `.o` files. Ghidra imports objects, not the `.a`. Extract outside the repository:

```bash
mkdir -p /tmp/zmod-liba && cd /tmp/zmod-liba
rm -f ./*.o
ar x /home/comet/renesas_zmod4510/lib/lib_no2_o3.a
file ./*.o
```

`lib_no2_o3.a` holds `nnet_funcs.c.o`, `no2_o3.c.o`, `no2_o3_NO2_nnet.c.o`, and `no2_o3_O3_nnet.c.o`. All four are ELF64 AArch64, not stripped, `GCC: (Linaro GCC 6.3-2017.02) 6.3.1 20170109`.

## 2. Post-script

The script is kept in the repository as [../../decompiled/scripts/DecompileAll.java](../../decompiled/scripts/DecompileAll.java). Copy it to `/tmp/zmod-ghidra/scripts/` or point `-scriptPath` at the repository directory. It writes `<outDir>/<object>.c` and drops the `.o` of the object name, so `nnet_funcs.c.o` gives `nnet_funcs.c`. Results go to `decompiled/lib_no2_o3/` and `decompiled/lib_zmod4xxx_cleaning/` (see [../../decompiled/README.md](../../decompiled/README.md)). The listing below is the same file.

```java
// Decompile every function of currentProgram into <outDir>/<object>.c
// (the ".o" of the object file name is dropped)
// Same job as tintinweb.vscode-decompiler scripts/ghidra_decompile.py, in Java
// because this Ghidra install has no Jython.
import java.io.File;
import java.io.PrintWriter;

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;

public class DecompileAll extends GhidraScript {
    @Override
    protected void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 1) {
            printerr("Wrong Arguments: need output directory");
            return;
        }
        File outDir = new File(args[0]);
        outDir.mkdirs();
        // "nnet_funcs.c.o" becomes "nnet_funcs.c", not "nnet_funcs.c.o.c"
        String name = currentProgram.getName();
        if (name.endsWith(".o")) {
            name = name.substring(0, name.length() - 2);
        }
        if (!name.endsWith(".c")) {
            name = name + ".c";
        }
        File out = new File(outDir, name);

        DecompInterface ifc = new DecompInterface();
        ifc.openProgram(currentProgram);

        try (PrintWriter w = new PrintWriter(out, "UTF-8")) {
            w.println("/* Program: " + currentProgram.getName() + " */");
            w.println("/* Language: " + currentProgram.getLanguageID() + " */");
            FunctionIterator it = currentProgram.getFunctionManager().getFunctions(true);
            while (it.hasNext()) {
                Function f = it.next();
                w.println();
                w.println("/* Function: " + f.getName() + " @ " + f.getEntryPoint() + " */");
                DecompileResults r = ifc.decompileFunction(f, 60, monitor);
                if (r != null && r.decompileCompleted() && r.getDecompiledFunction() != null) {
                    w.println(r.getDecompiledFunction().getC());
                } else {
                    w.println("/* decompile failed: " + (r == null ? "null" : r.getErrorMessage()) + " */");
                }
            }
        }
        println("WROTE " + out.getAbsolutePath());
        ifc.dispose();
    }
}
```

## 3. Run Ghidra headless

```bash
rm -rf /tmp/zmod-ghidra/proj /tmp/zmod-ghidra/out
mkdir -p /tmp/zmod-ghidra/proj /tmp/zmod-ghidra/out
cd /tmp/zmod-ghidra
nohup /home/comet/ghidra_12.2_PUBLIC/support/analyzeHeadless \
  /tmp/zmod-ghidra/proj zmod \
  -import /tmp/zmod-liba/nnet_funcs.c.o /tmp/zmod-liba/no2_o3.c.o \
          /tmp/zmod-liba/no2_o3_NO2_nnet.c.o /tmp/zmod-liba/no2_o3_O3_nnet.c.o \
  -scriptPath /tmp/zmod-ghidra/scripts \
  -postScript DecompileAll.java /tmp/zmod-ghidra/out \
  -deleteProject > /tmp/zmod-ghidra/run.log 2>&1 &
```

Ghidra detects `AARCH64:LE:64:v8A` itself. On the Raspberry Pi 4 the whole run took about 8 minutes. Startup is about 40 s and analysis is about 25 s per object, mostly in "Apply Data Archives" and "ASCII Strings". Output files appear one per object. Watch `/tmp/zmod-ghidra/run.log` for `REPORT: Analysis succeeded`. The run is finished when `pgrep -f analyzeHeadless` prints nothing.

`-deleteProject` leaves no Ghidra project behind. Ghidra still writes `~/.config/ghidra/ghidra_12.2_PUBLIC/` and caches under `/tmp/comet-ghidra` and `/var/tmp/comet-ghidra`.

## 4. Reading the output

- Each object is analyzed alone. A call into another object of the same archive (`nnet_funcs_dotbias`, `calc_no2_o3_O3_nnet`) or into libm (`exp`, `pow`, `log10`) is an unresolved external. Ghidra draws those stubs as `halt_baddata()` with `Bad instruction - Truncating control flow here`. The warning is about the stub, not about the algorithm.
- Constants are named `DAT_<address>` and are not shown as numbers. They live in `.rodata` of the same object. Dump them with `objdump -s -j .rodata /tmp/zmod-liba/<object>.o` and match the address. Those values are not extracted into the docs yet.
- Names such as `param_1` and `local_50` are Ghidra's. Match the field layout against `src/algos/no2_o3.h` and `src/sensors/zmod4xxx_types.h`. On AArch64 `zmod4xxx_dev_t` has `config[6]` at byte 1, `mox_er` at 8, `mox_lr` at 10, `pid` at 12, `prod_data` at 16.
- `nnet_funcs_relu` shows heavy NEON vectorization with `CONCAT13` and `SUB41` noise. It is `out[i] = in[i] > 0 ? in[i] : 0`.

## Limits

This is a static reading of one compiler's output. No sensor was running and nothing was executed. Names for constants and for the temperature input are inferred from usage and from the headers, not from Renesas source. The same steps were used for `lib/lib_zmod4xxx_cleaning.a`. It holds one object, `zmod4xxx_cleaning.c.o`, so replace the extract directory with `/tmp/zmod-libc`, pass that single file to `-import`, and use another project directory and output directory (`/tmp/zmod-ghidra/proj-c`, `out-c`). That run took about 1 minute. `objdump -dr` on the object is a useful cross-check for such a small file. The result is in [../decompiled-zmod4xxx-cleaning.md](../decompiled-zmod4xxx-cleaning.md).

Do not execute either archive's code to confirm a reading. `zmod4xxx_cleaning_run` writes to the sensor and can use up its one cleaning.
