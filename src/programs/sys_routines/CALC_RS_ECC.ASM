	CONST	N				16
	CONST	K				10	

	CONST	CSR_RSDAT		0x0D0
	CONST	CSR_RSDATRDY	0x0D1
	CONST	CSR_RSECCRDY	0x0D2
	CONST	CSR_RSADDR		0x0D3
	CONST	CSR_RSECC		0x0D4
	CONST	CSR_RSRST		0x0D5

	DB		DATBUF	N
	DB		ECCBUF	K

start:
	ADD		r1	r0	r0
	ADDI	r2	r0	16
	ISTORE	r2	r1(DATBUF)
	ADDI	r1	r1	1
	ADDI	r2	r0	32
	ISTORE	r2	r1(DATBUF)
	ADDI	r1	r1	1
	ADDI	r2	r0	12
	ISTORE	r2	r1(DATBUF)
	ADDI	r1	r1	1
	ADDI	r2	r0	86
	ISTORE	r2	r1(DATBUF)
	ADDI	r1	r1	1
	ADDI	r2	r0	97
	ISTORE	r2	r1(DATBUF)
	ADDI	r1	r1	1
	ADDI	r2	r0	128
	ISTORE	r2	r1(DATBUF)
	ADDI	r1	r1	1
	ADDI	r2	r0	236
	ISTORE	r2	r1(DATBUF)
	ADDI	r1	r1	1
	ADDI	r2	r0	17
	ISTORE	r2	r1(DATBUF)
	ADDI	r1	r1	1
	ADDI	r2	r0	236
	ISTORE	r2	r1(DATBUF)
	ADDI	r1	r1	1
	ADDI	r2	r0	17
	ISTORE	r2	r1(DATBUF)
	ADDI	r1	r1	1
	ADDI	r2	r0	236
	ISTORE	r2	r1(DATBUF)
	ADDI	r1	r1	1
	ADDI	r2	r0	17
	ISTORE	r2	r1(DATBUF)
	ADDI	r1	r1	1
	ADDI	r2	r0	236
	ISTORE	r2	r1(DATBUF)
	ADDI	r1	r1	1
	ADDI	r2	r0	17
	ISTORE	r2	r1(DATBUF)
	ADDI	r1	r1	1
	ADDI	r2	r0	236
	ISTORE	r2	r1(DATBUF)
	ADDI	r1	r1	1
	ADDI	r2	r0	17
	ISTORE	r2	r1(DATBUF)
	ADDI	r1	r1	1
	INVOKE	calc_rs_ecc
halt:
	JMP		halt

calc_rs_ecc:
	% Takes (8 * N + 7 * K + 10) CP
	% Size: 24 Insns (480 bits) - cheaper that soft impl.
	PUSH	r1
	PUSH	r2
	PUSH	r3
	ADDI	r1	r0	1
	OUTCSR	r1	CSR_RSRST
	OUTCSR	r0	CSR_RSRST
	ADDI	r1	r0	1
	ILOAD	r3	r0(DATBUF)
	% Stall
	ADDI	r2	r0	1
	OUTCSR	r3	CSR_RSDAT
	OUTCSR	r2	CSR_RSDATRDY
loop_calc_rs_ecc:
	% Loop body should have exactly 8 insns or bubbles
	% Otherwise we'll leave RSECC or CPU idle
	ILOAD	r3	r1(DATBUF)
	ADDI	r1	r1	1	% Exploiting the stall
	ADDI	r2	r1	-N
	OUTCSR	r3	CSR_RSDAT
	ADD		r0	r0	r0
	BNEZ	r2	loop_calc_rs_ecc
	% Stall before BNEZ branches
	% Stall before BNEZ branches
	
	ADDI	r1	r0	K-1
loop_read_rs_ecc:
	OUTCSR	r1	CSR_RSADDR
	INCSR	r2	CSR_RSECC
	ADDI	r1	r1	-1	% Exploit the stall 
	ISTORE	r2	r1(ECCBUF)
	BGEZ	r1	loop_read_rs_ecc
	% Stall before BNEZ branches
	% Stall before BNEZ branches
	POP		r3
	POP		r2
	POP		r1
	RET
