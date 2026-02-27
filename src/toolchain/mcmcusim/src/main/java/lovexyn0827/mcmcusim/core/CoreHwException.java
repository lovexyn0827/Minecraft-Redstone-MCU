package lovexyn0827.mcmcusim.core;

public class CoreHwException extends Exception {
	private static final long serialVersionUID = 943019097814330482L;
	
	CoreHwException(String string, Object ... args) {
		super(String.format(string, args));
	}
}
