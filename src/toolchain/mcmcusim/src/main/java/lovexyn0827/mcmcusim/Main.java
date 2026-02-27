package lovexyn0827.mcmcusim;

import java.lang.reflect.InvocationTargetException;

import javax.swing.SwingUtilities;

import lovexyn0827.mcmcusim.gui.MainFrame;

public class Main {
	// sim [filename]
	public static void main(String[] args) throws InvocationTargetException, InterruptedException {
		SwingUtilities.invokeAndWait(() -> {
			new MainFrame(args.length > 0 ? (String) args[0] : null);
		});
	}
}
