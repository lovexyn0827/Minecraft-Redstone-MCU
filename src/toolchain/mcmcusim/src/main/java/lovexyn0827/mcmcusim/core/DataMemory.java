package lovexyn0827.mcmcusim.core;

import java.util.Arrays;

public final class DataMemory {
	private static final int WORD_LEN = 8;
	private static final int CAPACITY = 256;
	private final int[] cells = new int[CAPACITY];
	
	public int read(int address) {
		assert address >= 0 && address < CAPACITY;
		return this.cells[address];
	}
	
	public void write(int address, int val) {
		assert address >= 0 && address < CAPACITY;
		assert (val & (-1 << WORD_LEN)) == 0;
		this.cells[address] = val;
	}
	
	public void reset() {
		Arrays.fill(this.cells, 0);
	}
}
