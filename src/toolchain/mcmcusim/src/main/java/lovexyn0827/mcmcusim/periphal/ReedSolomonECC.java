package lovexyn0827.mcmcusim.periphal;

import java.util.Arrays;

import it.unimi.dsi.fastutil.ints.IntSet;

public class ReedSolomonECC implements Periphal {
	private static final int RSCTL_CSR_NO = 0x0D0;
	private static final int RSDIN_CSR_NO = 0x0D1;
	private static final int RSADDR_CSR_NO = 0x0D2;
	private static final int RSECC_CSR_NO = 0x0D3;
	private static final IntSet CSR_NUMS = Periphal.buildCsrNumberSet(
			RSCTL_CSR_NO, RSDIN_CSR_NO, RSADDR_CSR_NO, RSECC_CSR_NO);
	
	private static final int ECCRDY_SHIFT = 1;
	private static final int DATRDY_SHIFT = 2;
	private static final int RST_SHIFT = 3;
	
	private byte rsCtl;
	private byte rsDin;
	private byte rsAddr;
	// private byte rsEcc; Implemented with combinatoric circuit
	
	private final int dataWordCount;
	private final int eccWordCount;
	private final byte[] generatorPolynomial;
	private final int fieldPrimitive;
	
	private final byte[] b;
	private final byte[] z;
	private byte multiplyStepCounter;
	private byte wordCounter;
	private byte x;
	
	public ReedSolomonECC(int dataWordCount, int eccWordCount, byte[] generatorPolynomial, int fieldPrimitive) {
		this.dataWordCount = dataWordCount;
		this.eccWordCount = eccWordCount;
		this.generatorPolynomial = generatorPolynomial;
		this.fieldPrimitive = fieldPrimitive;
		this.b = new byte[this.eccWordCount];
		this.z = new byte[this.eccWordCount];
		this.reset();
	}
	
	@Override
	public void tick() {
		if (this.isReady()) {
			return;
		}
		
		if ((this.rsCtl & (1 << RST_SHIFT)) != 0) {
			this.rsCtl &= ~(1 << DATRDY_SHIFT);
			this.implReset();
			return;
		}
		
		if (this.multiplyStepCounter == 0) {
			if ((this.rsCtl & (1 << DATRDY_SHIFT)) == 0) {
				return;
			}

			this.rsCtl &= ~(1 << DATRDY_SHIFT);
			this.wordCounter++;
			byte nextX = (byte) (this.b[0] ^ this.rsDin);
			for (int i = 0; i < this.eccWordCount; i++) {
				this.b[i] = i == 0 ? this.z[i] : (byte) (this.b[i - 1] ^ this.z[i]);
				this.z[i] = ((nextX & 0x80) != 0) ? this.generatorPolynomial[i] : 0;
			}
			
			this.multiplyStepCounter = 7;
			if (this.isReady()) {
				this.rsCtl |= 1 << ECCRDY_SHIFT;
			}
		} else {
			this.multiplyStepCounter--;
			for (int i = 0; i < this.eccWordCount; i++) {
				byte t = (byte) (this.z[i] << 1);
				if ((this.z[i] & 0x80) != 0) {
					t ^= this.fieldPrimitive;
				}
				
				if ((this.x & 0x80) != 0) {
					t ^= this.generatorPolynomial[i];
				}
				
				this.z[i] = t;
			}
			
			this.x = (byte) (this.x << 1);
		}
	}

	private boolean isReady() {
		return this.wordCounter >= this.dataWordCount;
	}

	@Override
	public void reset() {
		this.rsAddr = 0;
		this.rsCtl = 0;
		this.rsDin = 0;
		this.implReset();
	}

	private void implReset() {
		Arrays.fill(this.b, (byte) 0);
		Arrays.fill(this.z, (byte) 0);
		this.x = 0;
		this.multiplyStepCounter = 0;
		this.wordCounter = 0;
		this.rsCtl &= ~(1 << ECCRDY_SHIFT);
	}

	@Override
	public int readCsr(int csr) {
		switch (csr) {
		case RSADDR_CSR_NO:
			return this.rsAddr;
		case RSCTL_CSR_NO:
			return this.rsCtl;
		case RSDIN_CSR_NO:
			return this.rsDin;
		case RSECC_CSR_NO:
			return this.rsAddr < this.eccWordCount ? this.b[this.rsAddr] : 0;
		default:
			throw new AssertionError();
		}
	}

	@Override
	public void writeCsr(int csr, int val) {
		switch (csr) {
		case RSADDR_CSR_NO:
			this.rsAddr = (byte) val;
			break;
		case RSCTL_CSR_NO:
			this.rsCtl = (byte) val;
			break;
		case RSDIN_CSR_NO:
			this.rsDin = (byte) val;
			break;
		}
	}

	@Override
	public IntSet getCsrNumbers() {
		return CSR_NUMS;
	}

}
