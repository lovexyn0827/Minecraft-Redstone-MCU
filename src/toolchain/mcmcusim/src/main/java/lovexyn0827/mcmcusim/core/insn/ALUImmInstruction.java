package lovexyn0827.mcmcusim.core.insn;

import it.unimi.dsi.fastutil.ints.Int2ObjectMap;
import it.unimi.dsi.fastutil.ints.Int2ObjectOpenHashMap;
import lovexyn0827.mcmcusim.core.Core;

final class ALUImmInstruction extends Instruction {
	private static final Int2ObjectMap<String> INSN_NAMES;
	private static final Int2ObjectMap<String> INSN_NAMES_BITMANIP;
	private final int rs, rt, imm;
	private final ALUInstruction.Operation op;
	private final String insnName;
	private final String condCodeName;
	
	ALUImmInstruction(int insn) throws MalformedInstructionException {
		super(insn);
		int opcode = Instruction.getOpcode(insn);
		this.rs = Instruction.getRs(insn);
		this.rt = Instruction.getRt(insn);
		this.imm = Instruction.getImm8(insn);
		int funct;
		if (opcode == Opcodes.BITMANIP) {
			funct = Instruction.getFunct(insn);
			this.insnName = INSN_NAMES_BITMANIP.get(funct);
			this.condCodeName = null;
		} else if (opcode >= Opcodes.CMPIU_EQ & opcode <= Opcodes.CMPIU_LT) {
			funct = opcode | FunctCodes.CMPU_EQ;
			this.insnName = "CMPIU";
			this.condCodeName = ALUInstruction.COND_CODE_NAMES.get(opcode & 0x3);
		} else {
			funct = opcode ^ 0x8;
			this.insnName = INSN_NAMES.get(opcode);
			this.condCodeName = null;
		}
		
		this.op = ALUInstruction.OPERATION_BY_FUNC.get(funct);
		if (this.op == null) {
			throw new MalformedInstructionException("Unrecognized funct: %d", funct);
		}
	}

	@Override
	public void execute(Core context) {
		int leftOperand = context.getRegFile().read(this.rs);
		int rightOperand = this.imm;
		int result = this.op.apply(leftOperand, rightOperand);
		context.getRegFile().write(this.rt, result & 0xFF);
	}
	
	@Override
	public String deassemble() {
		if ("CMPIU".equals(this.insnName)) {
			return String.format("%s r%d r%d %s 0x%02x", 
					this.insnName, this.rt, this.rs, this.condCodeName, this.imm & 0xFF);
		} else {
			return String.format("%s r%d r%d 0x%02x", this.insnName, this.rt, this.rs, this.imm & 0xFF);
		}
	}
	
	static {
		INSN_NAMES = new Int2ObjectOpenHashMap<String>();
		INSN_NAMES.put(Opcodes.ILOAD, "ILOAD");
		INSN_NAMES.put(Opcodes.ISTORE, "ISTORE");
		INSN_NAMES.put(Opcodes.ADDI, "ADDI");
		INSN_NAMES.put(Opcodes.ANDI, "ANDI");
		INSN_NAMES.put(Opcodes.ORI, "ORI");
		INSN_NAMES.put(Opcodes.XORI, "XORI");
		
		INSN_NAMES_BITMANIP = new Int2ObjectOpenHashMap<String>();
		INSN_NAMES_BITMANIP.put(FunctCodes.SAR, "SARI");
		INSN_NAMES_BITMANIP.put(FunctCodes.SHL, "SHLI");
		INSN_NAMES_BITMANIP.put(FunctCodes.SHR, "SHRI");
		INSN_NAMES_BITMANIP.put(FunctCodes.SET, "SETI");
		INSN_NAMES_BITMANIP.put(FunctCodes.CLR, "CLRI");
		
		
	}
}
