package lovexyn0827.mcmcusim.core;

import java.util.Arrays;

public class RegFile {
	private static final int WORD_LEN = 8;
	private static final int CAPACITY = 16;
	private final int[] registers = new int[CAPACITY];
	
	public int read(int regNo) {
		assert regNo >= 0 && regNo < CAPACITY;
		if (regNo == 0) {
			return 0;
		} else {
			return this.registers[regNo];
		}
	}
	
	public void write(int regNo, int val) {
		assert regNo >= 0 && regNo < CAPACITY;
		assert (val & (-1 << WORD_LEN)) == 0;
		if (regNo != 0) {
			this.registers[regNo] = val;
		}
	}
	
	public void reset() {
		Arrays.fill(this.registers, 0);
	}
}
