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
