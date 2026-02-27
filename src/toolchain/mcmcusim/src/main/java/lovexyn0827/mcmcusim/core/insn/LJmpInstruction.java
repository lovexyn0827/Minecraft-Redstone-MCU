package lovexyn0827.mcmcusim.core.insn;

import lovexyn0827.mcmcusim.core.Core;

class LJmpInstruction extends Instruction {
	private final int rs, imm;
	
	LJmpInstruction(int insn) throws MalformedInstructionException {
		super(insn);
		this.rs = Instruction.getRs(insn);
		this.imm = Instruction.getImm12(insn);
	}

	@Override
	public void execute(Core context) {
		int rsVal = context.getRegFile().read(this.rs);
		int target = (rsVal + this.imm) & 0xFFF;
		context.jumpTo(target);
	}
	
	@Override
	public String deassemble() {
		return String.format("LJMP r%d(%d)", this.rs, this.imm);
	}
}
