package lovexyn0827.mcmcusim.core;

import it.unimi.dsi.fastutil.ints.Int2ObjectMap;
import it.unimi.dsi.fastutil.ints.Int2ObjectOpenHashMap;
import lovexyn0827.mcmcusim.core.insn.Instruction;
import lovexyn0827.mcmcusim.periphal.GPIO;
import lovexyn0827.mcmcusim.periphal.GlyphRom;
import lovexyn0827.mcmcusim.periphal.Periphal;
import lovexyn0827.mcmcusim.periphal.ReedSolomonECC;

public final class Core {
	private static final int PROG_ENTRY = 0x010;
	private int pc;
	private boolean shouldIncreasementPC = true;
	private final RegFile regFile;
	private final InstructionMemory insnMemory;
	private final DataMemory dataMemory;
	private final Stack callStack;
	private final Stack operandStack;
	private final Int2ObjectMap<Periphal> periphals;
	
	public Core() {
		this.regFile = new RegFile();
		this.insnMemory = new InstructionMemory();
		this.dataMemory = new DataMemory();
		this.callStack = new Stack(12, 8);
		this.operandStack = new Stack(8, 32);
		this.periphals = new Int2ObjectOpenHashMap<>();
		this.periphals.defaultReturnValue(Periphal.DUMMY);
		this.registerPeriphal(new GPIO());
		this.registerPeriphal(new ReedSolomonECC(3, 2, new byte[] { 2, 3, 1 }, 0x1D));
		this.registerPeriphal(new GlyphRom("D:/tom-thumb-new.png", 4, 6));
		this.reset();
	}
	
	public void tick() throws CoreHwException {
		Instruction insn = this.insnMemory.fetchInstruction(this.pc);
		this.shouldIncreasementPC = true;
		insn.execute(this);
		if (this.shouldIncreasementPC) {
			this.pc = (pc + 1) & 0xFFF;
		}

		for (Periphal periphal : this.periphals.values()) {
			periphal.tick();
		}
	}
	
	public void reset() {
		this.pc = PROG_ENTRY;
		this.regFile.reset();
		this.dataMemory.reset();
		this.callStack.reset();
		this.operandStack.reset();
		for (Periphal periphal : this.periphals.values()) {
			periphal.reset();
		}
	}

	public int getPC() {
		return this.pc;
	}

	public RegFile getRegFile() {
		return this.regFile;
	}

	public InstructionMemory getInsnMemory() {
		return this.insnMemory;
	}

	public DataMemory getDataMemory() {
		return this.dataMemory;
	}

	public Stack getCallStack() {
		return this.callStack;
	}

	public Stack getOperandStack() {
		return this.operandStack;
	}

	public Int2ObjectMap<Periphal> getPeriphals() {
		return this.periphals;
	}

	public void jumpTo(int target) {
		assert (target & (-1 << 12)) == 0;
		this.shouldIncreasementPC = false;
		this.pc = target & 0xFFF;
	}
	
	public void registerPeriphal(Periphal periphal) {
		for (int csr : periphal.getCsrNumbers()) {
			if (this.periphals.put(csr, periphal) != Periphal.DUMMY) {
				System.err.printf("CSR 0x%03d has already been assigned!\n", csr);
			}
		}
	}
}
