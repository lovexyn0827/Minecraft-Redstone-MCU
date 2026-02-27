package lovexyn0827.mcmcusim.core.insn;

public interface BTypeFunctCodes {
	static final int BEQZ = 0x0;
	static final int BNEZ = 0x1;
	static final int BGTZ = 0x2;
	static final int BLTZ = 0x3;

	static final int RET = 0x0;
	static final int JMP = 0x1;
	static final int INCSR = 0x2;
	static final int OUTCSR = 0x3;
}
