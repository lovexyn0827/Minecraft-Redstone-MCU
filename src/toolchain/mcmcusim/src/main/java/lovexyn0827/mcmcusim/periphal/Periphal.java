package lovexyn0827.mcmcusim.periphal;

import it.unimi.dsi.fastutil.ints.IntSet;
import it.unimi.dsi.fastutil.ints.IntSets;

public interface Periphal {
	static final Periphal DUMMY = new Dummy();
	
	void tick();
	void reset();
	int readCsr(int csr);
	void writeCsr(int csr, int val);
	IntSet getCsrNumbers();
	
	static final class Dummy implements Periphal {
		@Override
		public void tick() {
		}

		@Override
		public void reset() {
		}

		@Override
		public int readCsr(int csr) {
			return 0;
		}

		@Override
		public void writeCsr(int csr, int val) {
			// No-op, as expected
		}

		@Override
		public IntSet getCsrNumbers() {
			return IntSets.EMPTY_SET;
		}
	}
}
