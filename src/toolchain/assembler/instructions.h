typedef enum {
	R_TYPE,
	I_TYPE,
	B_TYPE,
	J_TYPE
} insn_type;

#define OP_INVALID (-1)

#define OP_ALU (0b0000)
#define OP_ADD OP_ALU
#define OP_SUB OP_ALU
#define OP_AND OP_ALU
#define OP_OR OP_ALU
#define OP_XOR OP_ALU
#define OP_SAR OP_ALU
#define OP_SHL OP_ALU
#define OP_SHR OP_ALU
#define OP_SET OP_ALU
#define OP_CLR OP_ALU
#define OP_PUSH OP_ALU
#define OP_POP OP_ALU
#define OP_CMPU OP_ALU

#define OP_BITMANIP (0b0001)
#define OP_SARI OP_BITMANIP
#define OP_SHLI OP_BITMANIP
#define OP_SHRI OP_BITMANIP
#define OP_SETI OP_BITMANIP
#define OP_CLRI OP_BITMANIP

#define OP_ILOAD (0b0010)
#define OP_ISTORE (0b0011)

#define OP_CMPIU_PREFIX (0b0100)
#define OP_CMPIU_EQ (0b0100)
#define OP_CMPIU_NE (0b0101)
#define OP_CMPIU_GT (0b0110)
#define OP_CMPIU_LT (0b0111)

#define OP_ADDI (0b1000)
#define OP_ANDI (0b1010)
#define OP_ORI (0b1011)
#define OP_XORI (0b1100)

#define OP_BRANCH (0b1110)
#define OP_BEQZ OP_BRANCH
#define OP_BNEZ OP_BRANCH
#define OP_BGTZ OP_BRANCH
#define OP_BLTZ OP_BRANCH

#define OP_MISCBTYPE (0b1111)
#define OP_RET OP_MISCBTYPE
#define OP_JMP OP_MISCBTYPE
#define OP_INCSR OP_MISCBTYPE
#define OP_OUTCSR OP_MISCBTYPE

#define OP_LJMP (0b1001)
#define OP_INVOKE (0b1101)
