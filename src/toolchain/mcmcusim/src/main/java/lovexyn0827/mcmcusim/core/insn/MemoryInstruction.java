package lovexyn0827.mcmcusim.core.insn;

import lovexyn0827.mcmcusim.core.Core;

final class MemoryInstruction extends Instruction {
	private final int rs, rt, imm;
	private final boolean write;
	
	MemoryInstruction(int insn) {
		super(insn);
		this.rs = Instruction.getRs(insn);
		this.rt = Instruction.getRt(insn);
		this.imm = Instruction.getImm8(insn);
		int opcode = Instruction.getOpcode(insn);
		if (opcode == Opcodes.ILOAD) {
			this.write = false;
		} else if (opcode == Opcodes.ISTORE) {
			this.write = true;
		} else {
			throw new IllegalArgumentException("Opcode must be ILOAD or ISTORE!");
		}
	}

	@Override
	public void execute(Core context) {
		int rsVal = context.getRegFile().read(this.rs);
		int address = (rsVal + this.imm) & 0xFF;
		if (this.write) {
			int rtVal = context.getRegFile().read(this.rt);
			context.getDataMemory().write(address, rtVal);
		} else {
			int memVal = context.getDataMemory().read(address);
			context.getRegFile().write(this.rt, memVal);
		}
	}
	
	@Override
	public String deassemble() {
		return String.format("%s r%d r%d(%d)", this.write ? "ISTORE" : "ILOAD", this.rt, this.rs, this.imm);
	}
}
