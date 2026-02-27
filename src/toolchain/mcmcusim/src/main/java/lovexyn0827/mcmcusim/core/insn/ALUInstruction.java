package lovexyn0827.mcmcusim.core.insn;

import it.unimi.dsi.fastutil.ints.Int2ObjectMap;
import it.unimi.dsi.fastutil.ints.Int2ObjectOpenHashMap;
import lovexyn0827.mcmcusim.core.Core;

final class ALUInstruction extends Instruction {
	static final Int2ObjectMap<Operation> OPERATION_BY_FUNC;
	private static final Int2ObjectMap<String> INSN_NAMES;
	static final Int2ObjectMap<String> COND_CODE_NAMES;
	private final int rs, rt, rd, funct;
	private final Operation op;
	
	ALUInstruction(int insn) throws MalformedInstructionException {
		super(insn);
		this.rs = Instruction.getRs(insn);
		this.rt = Instruction.getRt(insn);
		this.rd = Instruction.getRd(insn);
		this.funct = Instruction.getFunct(insn);
		this.op = OPERATION_BY_FUNC.get(this.funct);
		if (this.op == null) {
			throw new MalformedInstructionException("Unrecognized funct: %d", this.funct);
		}
	}
	
	@Override
	public void execute(Core context) {
		int leftOperand = context.getRegFile().read(this.rs);
		int rightOperand = context.getRegFile().read(this.rt);
		int result = this.op.apply(leftOperand, rightOperand);
		context.getRegFile().write(this.rd, result & 0xFF);
	}
	
	@Override
	public String deassemble() {
		String insnName = INSN_NAMES.get(this.funct);
		if ("CMPI".equals(insnName)) {
			String condCodeName = COND_CODE_NAMES.get(this.funct & 0x3);
			return String.format("%s r%d r%d %s r%d", 
					insnName, this.rd, this.rs, condCodeName, this.rt);
		} else {
			return String.format("%s r%d r%d r%d", insnName, this.rd, this.rs, this.rt);
		}
	}

	static {
		OPERATION_BY_FUNC = new Int2ObjectOpenHashMap<>();
		OPERATION_BY_FUNC.put(FunctCodes.ADD, (x, y) -> x + y);
		OPERATION_BY_FUNC.put(FunctCodes.SUB, (x, y) -> x - y);
		OPERATION_BY_FUNC.put(FunctCodes.AND, (x, y) -> x & y);
		OPERATION_BY_FUNC.put(FunctCodes.OR, (x, y) -> x | y);
		OPERATION_BY_FUNC.put(FunctCodes.XOR, (x, y) -> x ^ y);
		OPERATION_BY_FUNC.put(FunctCodes.SAR, (x, y) -> x >> (y & 7));
		OPERATION_BY_FUNC.put(FunctCodes.SHL, (x, y) -> x << (y & 7));
		OPERATION_BY_FUNC.put(FunctCodes.SHR, (x, y) -> x >>> (y & 7));
		OPERATION_BY_FUNC.put(FunctCodes.SET, (x, y) -> x | (1 << (y & 7)));
		OPERATION_BY_FUNC.put(FunctCodes.CLR, (x, y) -> x & ~(1 << (y & 7)));
		OPERATION_BY_FUNC.put(FunctCodes.CMPU_EQ, (x, y) -> x == y ? 1 : 0);
		OPERATION_BY_FUNC.put(FunctCodes.CMPU_NE, (x, y) -> x != y ? 1 : 0);
		OPERATION_BY_FUNC.put(FunctCodes.CMPU_GT, (x, y) -> x > y ? 1 : 0);
		OPERATION_BY_FUNC.put(FunctCodes.CMPU_LT, (x, y) -> x < y ? 1 : 0);
		
		INSN_NAMES = new Int2ObjectOpenHashMap<String>();
		INSN_NAMES.put(FunctCodes.ADD, "ADD");
		INSN_NAMES.put(FunctCodes.SUB, "SUB");
		INSN_NAMES.put(FunctCodes.AND, "AND");
		INSN_NAMES.put(FunctCodes.OR, "OR");
		INSN_NAMES.put(FunctCodes.XOR, "XOR");
		INSN_NAMES.put(FunctCodes.SAR, "SAR");
		INSN_NAMES.put(FunctCodes.SHL, "SHL");
		INSN_NAMES.put(FunctCodes.SHR, "SHR");
		INSN_NAMES.put(FunctCodes.SET, "SET");
		INSN_NAMES.put(FunctCodes.CLR, "CLR");
		INSN_NAMES.put(FunctCodes.CMPU_EQ, "CMPU");
		INSN_NAMES.put(FunctCodes.CMPU_NE, "CMPU");
		INSN_NAMES.put(FunctCodes.CMPU_GT, "CMPU");
		INSN_NAMES.put(FunctCodes.CMPU_LT, "CMPU");
		
		COND_CODE_NAMES = new Int2ObjectOpenHashMap<String>();
		COND_CODE_NAMES.put(0x0, "EQ");
		COND_CODE_NAMES.put(0x1, "NE");
		COND_CODE_NAMES.put(0x2, "GT");
		COND_CODE_NAMES.put(0x3, "LT");
	}
	
	static interface Operation {
		int apply(int x, int y);
	}
}
