package lovexyn0827.mcmcusim.core;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;

import lovexyn0827.mcmcusim.core.insn.Instruction;
import lovexyn0827.mcmcusim.core.insn.InstructionDecodeHelper;
import lovexyn0827.mcmcusim.core.insn.MalformedInstructionException;

public final class InstructionMemory {
	private static final int CAPACITY = 4096;
	
	private final Instruction[] content = new Instruction[CAPACITY];

	InstructionMemory() {
		this.clear();
		try {
			this.load(new File("D:/ASM_TMP"));// FIXME: Remove
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	private void clear() {
		try {
			Instruction nop = InstructionDecodeHelper.decode(0);
			Arrays.fill(this.content, nop);
		} catch (MalformedInstructionException e) {
			e.printStackTrace();
			throw new AssertionError(e);
		}
	}
	
	public Instruction fetchInstruction(int address) {
		assert address >= 0 && address < CAPACITY;
		return this.content[address];
	}
	
	public void load(File file) throws IOException {
		try (InputStream is = new BufferedInputStream(new FileInputStream(file))) {
			this.clear();
			int address = 0;
			do {
				// Little Endian
				int insn = is.read() | (is.read() << 8) | (is.read() << 16) | (is.read() << 24);
				//System.out.printf("Decoding 0x%08x\n", insn);
				try {
					this.content[address++] = InstructionDecodeHelper.decode(insn);
				} catch (MalformedInstructionException e) {
					System.err.printf("Error decoding 0x%05x: \n", insn);
					e.printStackTrace();
				}
			} while (is.available() > 0);
		}
	}

	public void setInstruction(int address, int insn) throws MalformedInstructionException {
		this.content[address] = InstructionDecodeHelper.decode(insn);
	}
}
