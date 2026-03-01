package lovexyn0827.mcmcusim.core;

import it.unimi.dsi.fastutil.ints.IntArrayList;

public final class Stack {
	private final IntArrayList backend;
	private final int wordLen;
	private final int capacity;
	
	Stack(int wordLen, int capacity) {
		this.backend = new IntArrayList();
		this.wordLen = wordLen;
		this.capacity = capacity;
	}
	
	public void push(int val) throws CoreHwException {
		assert (val & (-1 << this.wordLen)) == 0;
		if (this.backend.size() == this.capacity) {
			throw new CoreHwException("Stack overflowed");
		}
		
		this.backend.push(val);
	}
	
	public int pop() throws CoreHwException {
		if (this.backend.isEmpty()) {
			throw new CoreHwException("Stack underflowed");
		}
		
		return this.backend.popInt();
	}

	public void reset() {
		this.backend.clear();
	}

	public int debugGet(int row) {
		return row < this.backend.size() ? this.backend.getInt(row) : 0;
	}

	public void debugSet(int row, int newVal) {
		if (row < this.backend.size()) {
			this.backend.set(row, newVal);
		}
	}

	public int getWordLength() {
		return this.wordLen;
	}

	public int getCapacity() {
		return this.capacity;
	}

	public int debugGetPointer() {
		return this.backend.size();
	}
}
