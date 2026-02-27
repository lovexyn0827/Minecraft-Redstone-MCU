package lovexyn0827.mcmcusim.core.insn;

import lovexyn0827.mcmcusim.core.Core;

final class IOInstruction extends Instruction {
	private final int rs, imm;
	private final boolean write;
	
	IOInstruction(int insn) {
		super(insn);
		this.rs = Instruction.getRs(insn);
		this.imm = Instruction.getImm10(insn);
		int bTypeFunct = Instruction.getBTypeFunct(insn);
		if (bTypeFunct == BTypeFunctCodes.INCSR) {
			this.write = false;
		} else if (bTypeFunct == BTypeFunctCodes.OUTCSR) {
			this.write = true;
		} else {
			throw new IllegalArgumentException("BTFunct must be INCSR or OUTCSR!");
		}
	}

	@Override
	public void execute(Core context) {
		if (this.write) {
			int rtVal = context.getRegFile().read(this.rs);
			context.getPeriphals().get(this.imm).writeCsr(this.imm, rtVal);
		} else {
			int memVal = context.getPeriphals().get(this.imm).readCsr(this.imm);
			context.getRegFile().write(this.rs, memVal);
		}
	}
	
	@Override
	public String deassemble() {
		return String.format("%s r%d r0x%03x", this.write ? "INCSR" : "OUTCSR", this.rs, this.imm);
	}
}
