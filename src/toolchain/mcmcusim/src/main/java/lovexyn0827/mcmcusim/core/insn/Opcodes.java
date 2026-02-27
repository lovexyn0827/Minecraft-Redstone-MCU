package lovexyn0827.mcmcusim.core.insn;

public interface Opcodes {
	static int RTYPE = 0x0;
	static int BITMANIP = 0x1;
	static int ILOAD = 0x2;
	static int ISTORE = 0x3;
	static int CMPIU_EQ = 0x4;
	static int CMPIU_NE = 0x5;
	static int CMPIU_GT = 0x6;
	static int CMPIU_LT = 0x7;
	static int ADDI = 0x8;
	static int LJMP = 0x9;
	static int ANDI = 0xA;
	static int ORI = 0xB;
	static int XORI = 0xC;
	static int INVOKE = 0xD;
	static int BTYPE1 = 0xE;
	static int BTYPE2 = 0xF;
}
