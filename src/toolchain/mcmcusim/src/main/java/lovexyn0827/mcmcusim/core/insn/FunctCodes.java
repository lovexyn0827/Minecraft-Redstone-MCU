package lovexyn0827.mcmcusim.core.insn;

public interface FunctCodes {
	static final int ADD = 0x0;
	static final int SUB = 0x1;
	static final int AND = 0x2;
	static final int OR = 0x3;
	static final int XOR = 0x4;
	static final int SAR = 0x5;
	static final int SHL = 0x6;
	static final int SHR = 0x7;
	static final int SET = 0x8;
	static final int CLR = 0x9;
	static final int PUSH = 0xA;
	static final int POP = 0xB;
	static final int CMPU_EQ = 0xC;
	static final int CMPU_NE = 0xD;
	static final int CMPU_GT = 0xE;
	static final int CMPU_LT = 0xF;
}
