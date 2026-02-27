package lovexyn0827.mcmcusim.core.insn;

import lovexyn0827.mcmcusim.core.Core;
import lovexyn0827.mcmcusim.core.CoreHwException;

final class PushInstruction extends Instruction {
	private final int rs;

	PushInstruction(int insn) {
		super(insn);
		this.rs = Instruction.getRs(insn);
	}

	@Override
	public void execute(Core context) throws CoreHwException {
		int rsVal = context.getRegFile().read(this.rs);
		context.getOperandStack().push(rsVal);
	}
	
	@Override
	public String deassemble() {
		return String.format("PUSH r%d", this.rs);
	}
}
