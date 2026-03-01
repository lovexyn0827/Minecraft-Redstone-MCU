package lovexyn0827.mcmcusim.periphal;

import it.unimi.dsi.fastutil.ints.IntSet;

public class GPIO implements Periphal {
	private static final int GPIBUF_CSR_NO = 0x08E;
	private static final int GPOBUF_CSR_NO = 0x08F;
	private static final IntSet CSR_NUMS;
	
	private byte gpiBuf = 0;
	private byte gpoBuf = 0;
	private byte transientInput = 0;

	@Override
	public void tick() {
		this.gpiBuf = this.transientInput;
	}

	@Override
	public void reset() {
		this.gpiBuf = 0;
		this.gpoBuf = 0;
	}

	@Override
	public int readCsr(int csr) {
		if (csr == GPIBUF_CSR_NO) {
			return this.gpiBuf;
		} else if (csr == GPOBUF_CSR_NO) {
			return this.gpoBuf;
		} else {
			throw new AssertionError();
		}
	}

	@Override
	public void writeCsr(int csr, int val) {
		assert (val & (-1 << 8)) == 0;
		if (csr == GPOBUF_CSR_NO) {
			this.gpoBuf = (byte) val;
		}
	}

	@Override
	public IntSet getCsrNumbers() {
		return CSR_NUMS;
	}
	
	public void setInput(int val) {
		assert (val & (-1 << 8)) == 0;
		this.transientInput  = (byte) val;
	}
	
	public int getOutput() {
		return this.gpoBuf;
	}

	static {
		CSR_NUMS = Periphal.buildCsrNumberSet(GPIBUF_CSR_NO, GPOBUF_CSR_NO);
	}
}
