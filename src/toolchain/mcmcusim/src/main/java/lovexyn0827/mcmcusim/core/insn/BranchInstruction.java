package lovexyn0827.mcmcusim.core.insn;

import it.unimi.dsi.fastutil.ints.Int2ObjectMap;
import it.unimi.dsi.fastutil.ints.Int2ObjectOpenHashMap;
import lovexyn0827.mcmcusim.core.Core;

final class BranchInstruction extends Instruction {
	private static final Int2ObjectMap<String> INSN_NAMES;
	private final int rs, imm, bTypeFunct;
	
	BranchInstruction(int insn) throws MalformedInstructionException {
		super(insn);
		this.rs = Instruction.getRs(insn);
		this.imm = Instruction.getImm10(insn);
		this.bTypeFunct = Instruction.getBTypeFunct(insn);
	}

	@Override
	public void execute(Core context) {
		int rsVal = context.getRegFile().read(this.rs);
		boolean shouldBranch;
		switch (this.bTypeFunct) {
		case 0x0:
			shouldBranch = rsVal == 0;
			break;
		case 0x1:
			shouldBranch = rsVal != 0;
			break;
		case 0x2:
			shouldBranch = (rsVal) <= 0x7F;
			break;
		case 0x3:
			shouldBranch = rsVal >= 0x80;
			break;
		default:
			throw new IllegalStateException();
		}
		
		if (shouldBranch) {
			int target = (context.getPC() + this.imm) & 0xFFF;
			context.jumpTo(target);
		}
	}
	
	@Override
	public String deassemble() {
		String insnName = INSN_NAMES.get(this.bTypeFunct);
		return String.format("%d r%d %d", insnName, this.rs, this.imm);
	}
	
	@Override
	public String toString() {
		return this.deassemble();
	}
	
	static {
		INSN_NAMES = new Int2ObjectOpenHashMap<String>();
		INSN_NAMES.put(BTypeFunctCodes.BEQZ, "BEQZ");
		INSN_NAMES.put(BTypeFunctCodes.BNEZ, "BNEZ");
		INSN_NAMES.put(BTypeFunctCodes.BGTZ, "BGTZ");
		INSN_NAMES.put(BTypeFunctCodes.BLTZ, "BLTZ");
	}
}
