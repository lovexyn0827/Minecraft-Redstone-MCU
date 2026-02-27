package lovexyn0827.mcmcusim.core.insn;

import lovexyn0827.mcmcusim.core.Core;
import lovexyn0827.mcmcusim.core.CoreHwException;

final class RetInstruction extends Instruction {
	private final boolean iret;
	
	RetInstruction(int insn) {
		super(insn);
		this.iret = Instruction.getImm10(insn) == 0;
	}
	
	@Override
	public void execute(Core context) throws CoreHwException {
		int retAddr = (context.getCallStack().pop() + (this.iret ? 0 : 1)) & 0xFFF;
		context.jumpTo(retAddr);
	}
	
	@Override
	public String deassemble() {
		return this.iret ? "RET I" : "RET";
	}
}
