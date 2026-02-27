package lovexyn0827.mcmcusim.core.insn;

import lovexyn0827.mcmcusim.core.Core;
import lovexyn0827.mcmcusim.core.CoreHwException;

final class PopInstruction extends Instruction {
	private final int rd;

	PopInstruction(int insn) {
		super(insn);
		this.rd = Instruction.getRd(insn);
	}

	@Override
	public void execute(Core context) throws CoreHwException {
		int stackTop = context.getOperandStack().pop();
		context.getRegFile().write(this.rd, stackTop);
	}
	
	@Override
	public String deassemble() {
		return String.format("POP r%d", this.rd);
	}
}
