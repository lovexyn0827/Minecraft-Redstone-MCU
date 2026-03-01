package lovexyn0827.mcmcusim.core.insn;

import lovexyn0827.mcmcusim.core.Core;

class JmpInstruction extends Instruction {
	private final int imm;
	
	JmpInstruction(int insn) throws MalformedInstructionException {
		super(insn);
		this.imm = Instruction.getImm10(insn);
	}

	@Override
	public void execute(Core context) {
		int target = (context.getPC() + Instruction.signExt(this.imm, 10)) & 0xFFF;
		context.jumpTo(target);
	}
	
	@Override
	public String deassemble() {
		return String.format("JMP %d", Instruction.signExt(this.imm, 10));
	}
}
