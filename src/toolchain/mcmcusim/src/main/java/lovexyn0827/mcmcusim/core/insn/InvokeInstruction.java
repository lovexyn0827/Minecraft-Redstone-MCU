package lovexyn0827.mcmcusim.core.insn;

import lovexyn0827.mcmcusim.core.Core;
import lovexyn0827.mcmcusim.core.CoreHwException;

final class InvokeInstruction extends Instruction {
	private final int rs, imm;
	
	InvokeInstruction(int insn) {
		super(insn);
		this.rs = Instruction.getRs(insn);
		this.imm = Instruction.getImm12(insn);
	}

	@Override
	public void execute(Core context) throws CoreHwException {
		int prevPC = context.getPC();
		context.getCallStack().push(prevPC);
		int rsVal = context.getRegFile().read(this.rs);
		int target = (rsVal + this.imm) & 0xFFF;
		context.jumpTo(target);
		
	}
	
	@Override
	public String deassemble() {
		return String.format("INVOKE r%d(%d)", this.rs, this.imm);
	}
}
