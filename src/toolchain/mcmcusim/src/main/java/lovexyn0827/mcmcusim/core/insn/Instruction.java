package lovexyn0827.mcmcusim.core.insn;

import lovexyn0827.mcmcusim.core.Core;
import lovexyn0827.mcmcusim.core.CoreHwException;

public abstract class Instruction {
	private final int binary;
	
	Instruction(int binary) {
		this.binary = binary;
	}
	
	public int getBinary() {
		return this.binary;
	}
	
	public abstract void execute(Core context) throws CoreHwException;
	public abstract String deassemble();
	
	@Override
	public String toString() {
		return this.deassemble();
	}
	
	@Override
	public boolean equals(Object obj) {
		if (!(obj instanceof Instruction)) {
			return false;
		}
		
		return this.binary == ((Instruction) obj).binary;
	}
	
	@Override
	public int hashCode() {
		return this.binary;
	}
	
	private static int clipBitRange(int in, int from, int len) {
		return (in >> from) & ((1 << len) - 1);
	}
	
	private static int signExt(int in, int inBits) {
		return in | (-1 << inBits);
	}

	static int getOpcode(int insn) {
		return clipBitRange(insn, 16, 4);
	}
	
	static int getRs(int insn) {
		return clipBitRange(insn, 12, 4);
	}
	
	static int getRt(int insn) {
		return clipBitRange(insn, 8, 4);
	}
	
	static int getRd(int insn) {
		return clipBitRange(insn, 4, 4);
	}
	
	static int getFunct(int insn) {
		return clipBitRange(insn, 0, 4);
	}
	
	static int getImm3(int insn) {
		return signExt(clipBitRange(insn, 5, 3), 3);
	}
	
	static int getImm8(int insn) {
		return signExt(clipBitRange(insn, 0, 8), 8);
	}
	
	static int getImm10(int insn) {
		return signExt(clipBitRange(insn, 0, 10), 10);
	}
	
	static int getImm12(int insn) {
		return signExt(clipBitRange(insn, 0, 12), 12);
	}
	
	static int getBTypeFunct(int insn) {
		return clipBitRange(insn, 10, 2);
	}
}
