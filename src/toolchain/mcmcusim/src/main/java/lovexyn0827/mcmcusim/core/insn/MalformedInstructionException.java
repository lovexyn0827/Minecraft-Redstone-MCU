package lovexyn0827.mcmcusim.core.insn;

public class MalformedInstructionException extends Exception {
	private static final long serialVersionUID = -2990890605501780928L;

	MalformedInstructionException(String string, Object ... args) {
		super(String.format(string, args));
	}
}
