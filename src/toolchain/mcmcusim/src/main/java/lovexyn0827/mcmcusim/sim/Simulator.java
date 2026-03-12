package lovexyn0827.mcmcusim.sim;

import java.awt.EventQueue;
import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.util.Timer;
import java.util.TimerTask;
import java.util.function.BiConsumer;
import java.util.function.Consumer;

import javax.swing.SwingUtilities;

import lovexyn0827.mcmcusim.core.Core;
import lovexyn0827.mcmcusim.core.CoreHwException;

public final class Simulator {
	private final Core core = new Core();
	private boolean running = false;
	private boolean superHot = false;
	private Timer tickTimer = new Timer();
	private Timer ipsTimer = new Timer();
	private int tickPeriod = 50;
	private int prevIps;
	private int ticksSinceLastSecond;
	private boolean[] breakPoints = new boolean[4096];
	private boolean resumingFromBreakPoint = false;
	private TimerTask currentTickTask;
	private File prevProgImage;
	private final Consumer<Simulator> guiRefreshCallback;
	private final BiConsumer<Simulator, String> errorCallback;
	
	public Simulator(Consumer<Simulator> guiCallback, BiConsumer<Simulator, String> errorCallback) {
		this.guiRefreshCallback = guiCallback;
		this.errorCallback = errorCallback;
		this.ipsTimer.schedule(new TimerTask() {
			@Override
			public void run() {
				Simulator.this.prevIps = Simulator.this.ticksSinceLastSecond;
				Simulator.this.ticksSinceLastSecond = 0;
			}
		}, 0, 1000);
	}
	
	public int getIps() {
		return this.prevIps;
	}

	public void loadProgram(File file) {
		this.prevProgImage = file;
		this.reloadProgram();
	}
	
	public void reloadProgram() {
		try {
			this.core.getInsnMemory().load(this.prevProgImage);
		} catch (IOException e) {
			this.errorCallback.accept(this, e.getLocalizedMessage());
			e.printStackTrace();
		}
		
		this.core.reset();
	}
	
	public Core getCore() {
		return this.core;
	}
	
	public boolean[] getBreakPoints() {
		return this.breakPoints;
	}
	
	public int getTickPeriod() {
		return this.tickPeriod;
	}
	
	public boolean isSuperHot() {
		return this.superHot;
	}
	
	public boolean isRunning() {
		return this.running;
	}
	
	public void setSuperHot(boolean superHot) {
		this.superHot = superHot;
		this.resume();
	}
	
	public void setRunning(boolean running) {
		this.running = running;
		if (this.running) {
			this.resumingFromBreakPoint = true;
			this.resume();
		}
	}
	
	public void step() {
		this.resumingFromBreakPoint = true;
		this.tick();
	}
	
	public void reset() {
		this.core.reset();
		this.running = false;
		this.guiRefreshCallback.accept(this);
	}
	
	public void setTickPeriod(int period) {
		this.tickPeriod = period;
		this.resume();
	}
	
	public void resume() {
		if (this.currentTickTask != null) {
			this.currentTickTask.cancel();
			this.tickTimer.purge();
		}
		
		this.currentTickTask = this.createTickTask();
		this.tickTimer.schedule(this.currentTickTask, 0, this.tickPeriod);
	}

	private TimerTask createTickTask() {
		return new TimerTask() {
			@Override
			public void run() {
				if (Simulator.this.running) {
					int loopCount = Simulator.this.superHot ? 1000 * Simulator.this.tickPeriod : 1;
					for (int i = 0; i < loopCount; i++) {
						if (Simulator.this.tick()) {
							Simulator.this.guiRefreshCallback.accept(Simulator.this);
							break;
						}
					}
				} else {
					this.cancel();
				}
			}
		};
	}

	private boolean tick() {
		if (EventQueue.isDispatchThread()) {
			if (this.detectBreakpoint()) {
				return true;
			}
			
			try {
				this.core.tick();
				this.ticksSinceLastSecond++;
			} catch (CoreHwException e) {
				this.errorCallback.accept(this, e.getLocalizedMessage());
				this.running = false;
				e.printStackTrace();
			}
			
			if (!this.superHot) {
				this.guiRefreshCallback.accept(this);;
			}
			
			return false;
		} else {
			try {
				boolean hasBreakpoint[] = new boolean[] { false } ;
				SwingUtilities.invokeAndWait(() -> {
					hasBreakpoint[0] = this.tick();
				});
				return hasBreakpoint[0];
			} catch (InvocationTargetException | InterruptedException e) {
				e.printStackTrace();
				return false;
			}
		}
	}

	private boolean detectBreakpoint() {
		if (this.resumingFromBreakPoint) {
			this.resumingFromBreakPoint = false;
			return false;
		}
		
		if (this.breakPoints[this.core.getPC()]) {
			this.running = false;
			return true;
		} else {
			return false;
		}
	}
}
