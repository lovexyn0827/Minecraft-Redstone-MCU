package lovexyn0827.mcmcusim.core.insn;

public interface InstructionDecodeHelper {
	static Instruction decode(int insn) throws MalformedInstructionException {
		assert (insn & (-1 << 20)) == 0;
		int opcode = Instruction.getOpcode(insn);
		int funct = Instruction.getFunct(insn);
		int bTypeFunct = Instruction.getBTypeFunct(insn);
		switch (opcode) {
		case Opcodes.RTYPE:
			switch (funct) {
			case FunctCodes.PUSH:
				return new PushInstruction(insn);
			case FunctCodes.POP:
				return new PopInstruction(insn);
			default:
				return new ALUInstruction(insn);
			}
		case Opcodes.BTYPE1:
			return new BranchInstruction(insn);
		case Opcodes.BTYPE2:
			switch (bTypeFunct) {
			case BTypeFunctCodes.RET:
				return new RetInstruction(insn);
			case BTypeFunctCodes.JMP:
				return new JmpInstruction(insn);
			case BTypeFunctCodes.INCSR:
			case BTypeFunctCodes.OUTCSR:
				return new IOInstruction(insn);
			}
		case Opcodes.LJMP:
			return new LJmpInstruction(insn);
		case Opcodes.INVOKE:
			return new InvokeInstruction(insn);
		case Opcodes.ILOAD:
		case Opcodes.ISTORE:
			return new MemoryInstruction(insn);
		default:
			return new ALUImmInstruction(insn);
		}
	}
}
