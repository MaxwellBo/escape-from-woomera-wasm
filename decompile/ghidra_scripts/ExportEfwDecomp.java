// Decompile every function and also emit an EFW-focused subset.
//@category EFW
import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Pattern;

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileOptions;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.DataIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolIterator;
import ghidra.program.model.symbol.SymbolTable;
import ghidra.program.model.symbol.SymbolType;

public class ExportEfwDecomp extends GhidraScript {

	private static final Pattern EFW = Pattern.compile(
		"(?i)efw|refugee|conversation|squark|hope|diary|pliers|marker|washing|idtag|hideunder");

	@Override
	public void run() throws Exception {
		String outDir = getScriptArgs().length > 0 ? getScriptArgs()[0] : "/workspace/decompile/out";
		String stem = currentProgram.getName().replace(".dll", "");
		new File(outDir).mkdirs();

		File indexFile = new File(outDir, stem + "_functions.txt");
		File allFile = new File(outDir, stem + "_ghidra.c");
		File efwFile = new File(outDir, stem + "_ghidra_efw.c");

		DecompInterface decomp = new DecompInterface();
		DecompileOptions opts = new DecompileOptions();
		decomp.setOptions(opts);
		decomp.openProgram(currentProgram);

		Set<Function> interesting = new HashSet<Function>();
		SymbolTable st = currentProgram.getSymbolTable();
		SymbolIterator sit = st.getAllSymbols(true);
		while (sit.hasNext() && !monitor.isCancelled()) {
			Symbol sym = sit.next();
			if (sym.getSymbolType() != SymbolType.LABEL && sym.getSymbolType() != SymbolType.FUNCTION) {
				continue;
			}
			String name = sym.getName(true);
			if (!EFW.matcher(name).find()) {
				continue;
			}
			Function f = getFunctionContaining(sym.getAddress());
			if (f != null) {
				interesting.add(f);
			}
			for (Reference ref : getReferencesTo(sym.getAddress())) {
				Function rf = getFunctionContaining(ref.getFromAddress());
				if (rf != null) {
					interesting.add(rf);
				}
			}
		}
		DataIterator dit = currentProgram.getListing().getDefinedData(true);
		while (dit.hasNext() && !monitor.isCancelled()) {
			Data d = dit.next();
			if (!d.hasStringValue()) {
				continue;
			}
			String val = d.getDefaultValueRepresentation();
			if (val == null || !EFW.matcher(val).find()) {
				continue;
			}
			for (Reference ref : getReferencesTo(d.getAddress())) {
				Function rf = getFunctionContaining(ref.getFromAddress());
				if (rf != null) {
					interesting.add(rf);
				}
			}
		}
		println("interesting functions: " + interesting.size());
		String prog = currentProgram.getName();
		FunctionIterator rangeIt = currentProgram.getFunctionManager().getFunctions(true);
		while (rangeIt.hasNext()) {
			Function fn = rangeIt.next();
			long off = fn.getEntryPoint().getOffset();
			// Original EFW overlay cluster (CRefugee, efw_Marker, weapon_efw_*).
			if (prog.contains("EscapeFromWoomera") && off >= 0x100c0000L && off < 0x100c84b0L) {
				interesting.add(fn);
			}
			// ClientCommand / say dispatcher lives below the overlay cluster.
			if (prog.contains("EscapeFromWoomera") && off >= 0x1001a000L && off < 0x1001c000L) {
				interesting.add(fn);
			}
			// Client HUD entry points.
			String n = fn.getName(true);
			if (n.startsWith("HUD_") || n.equals("Initialize") || n.contains("weapon_efw")) {
				interesting.add(fn);
			}
		}
		println("interesting + overlay cluster: " + interesting.size());

		PrintWriter index = new PrintWriter(new FileWriter(indexFile));
		PrintWriter all = new PrintWriter(new FileWriter(allFile));
		PrintWriter efw = new PrintWriter(new FileWriter(efwFile));
		all.println("/* Ghidra decompile of " + currentProgram.getName() + " */");
		efw.println("/* EFW-focused Ghidra decompile of " + currentProgram.getName() + " */");

		FunctionIterator it = currentProgram.getFunctionManager().getFunctions(true);
		int n = 0;
		int nEfw = 0;
		while (it.hasNext() && !monitor.isCancelled()) {
			Function fn = it.next();
			n++;
			index.printf("%s\t%s\t%d%n", fn.getEntryPoint(), fn.getName(true), fn.getBody().getNumAddresses());
			boolean want = interesting.contains(fn) || EFW.matcher(fn.getName(true)).find();
			DecompileResults res = decomp.decompileFunction(fn, 60, monitor);
			String code = (res != null && res.decompileCompleted()) ? res.getDecompiledFunction().getC()
				: "/* decompile failed: " + (res == null ? "null" : res.getErrorMessage()) + " */\n";
			all.println();
			all.println("/* ==== " + fn.getEntryPoint() + " " + fn.getName(true) + " ==== */");
			all.println(code);
			if (want || codeMatches(code)) {
				nEfw++;
				efw.println();
				efw.println("/* ==== " + fn.getEntryPoint() + " " + fn.getName(true) + " ==== */");
				efw.println(code);
			}
			if (n % 100 == 0) {
				println("decompiled " + n + " functions, efw=" + nEfw);
			}
		}
		index.close();
		all.close();
		efw.close();
		decomp.dispose();
		println("wrote " + n + " functions, " + nEfw + " EFW-focused, to " + outDir);
	}

	private boolean codeMatches(String code) {
		return EFW.matcher(code).find();
	}
}
