package lovexyn0827.mcmcusim.gui;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.FlowLayout;
import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.util.Timer;
import java.util.TimerTask;

import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.SwingUtilities;
import lovexyn0827.mcmcusim.core.Core;
import lovexyn0827.mcmcusim.core.CoreHwException;

public class MainFrame extends JFrame {
	private static final long serialVersionUID = 3621344886019049445L;
	private InstructionTable insnTable;
	private SramTable ramTable;
	private RegisterTable regTable;
	private StackTable stackTable;
	private CSRTable csrTable;
	private VirtualDisplay display;
	private JScrollPane insnScrollPane;
	private JScrollPane ramScrollPane;
	private JScrollPane registerScrollPane;
	private JScrollPane stackScrollPane;
	private JScrollPane csrScrollPane;
	private Core core = new Core();
	private boolean running = false;
	private boolean superHot = false;
	private Timer tickTimer = new Timer();
	private Timer ipsTimer = new Timer();
	private int tickPeriod = 50;
	private int ticksSinceLastSecond;
	private JLabel statusLabel;
	private boolean[] breakPoints = new boolean[4096];
	private boolean resumingFromBreakPoint = false;
	private TimerTask currentTickTask;
	private File prevProgImage;
	
	public MainFrame(String fileName) {
		super();
		if (fileName != null) {
			try {
				this.prevProgImage = new File(fileName);
				this.core.getInsnMemory().load(this.prevProgImage);
			} catch (IOException e) {
				System.err.println("Failed to open image file!");
				e.printStackTrace();
				// Exit if the file name is mispecified from CLI.
				System.exit(1);
			}
		}

		this.init();
		this.setVisible(true);
		this.reload();
	}
	
	/* |=============================|
	 * |       Debugging Tools       |
	 * |=============================|
	 * |       |          |          |
	 * |       |   SRAM   |   CSR    |
	 * | Insns |=====================|
	 * |       | Reg |  S |   Disp   |
	 * |       |     |  S |          |
	 * |=============================|
	 */
	
	void init() {
		this.setLayout(new BorderLayout(5, 5));
		this.setSize(1440, 720);
		this.setDefaultCloseOperation(EXIT_ON_CLOSE);
		
		// Tools
		JPanel toolsPanel = new JPanel();
		toolsPanel.setLayout(new FlowLayout(5));
		this.add(toolsPanel, BorderLayout.NORTH);
		
		// Inspect Windows & Insns
		JPanel inspectPanel = new JPanel();
		this.insnScrollPane = new JScrollPane(this.insnTable = new InstructionTable());
		inspectPanel.setLayout(new BorderLayout(5, 5));
		inspectPanel.add(this.insnScrollPane, BorderLayout.WEST);
		this.add(inspectPanel, BorderLayout.SOUTH);
		
		// Right half
		JPanel rightInspectPanel = new JPanel();
		rightInspectPanel.setLayout(new BorderLayout(5, 5));
		inspectPanel.add(rightInspectPanel, BorderLayout.EAST);
		
		// Regs & Stack & Disp
		JPanel bottomInspectPanel = new JPanel();
		bottomInspectPanel.setLayout(new BorderLayout(5, 5));
		this.registerScrollPane = new JScrollPane(this.regTable = new RegisterTable());
		bottomInspectPanel.add(this.registerScrollPane, BorderLayout.WEST);
		this.stackScrollPane = new JScrollPane(this.stackTable = new StackTable());
		bottomInspectPanel.add(this.stackScrollPane);
		bottomInspectPanel.add(this.display = new VirtualDisplay(), BorderLayout.EAST);
		rightInspectPanel.add(bottomInspectPanel, BorderLayout.SOUTH);
		
		// RAM Panel & CSRs
		JPanel topInspectPanel = new JPanel();
		topInspectPanel.setLayout(new BorderLayout(5, 5));
		this.ramScrollPane = new JScrollPane(this.ramTable = new SramTable(this.display));
		topInspectPanel.add(this.ramScrollPane, BorderLayout.WEST);
		this.csrScrollPane = new JScrollPane(this.csrTable = new CSRTable());
		topInspectPanel.add(this.csrScrollPane, BorderLayout.EAST);
		rightInspectPanel.add(topInspectPanel, BorderLayout.NORTH);
		
		// Buttons
		JButton loadBtn = new JButton("Load");
		loadBtn.addActionListener((ae) -> {
			JFileChooser fileChooser = new JFileChooser();
			fileChooser.showOpenDialog(this);
			File image = fileChooser.getSelectedFile();
			if (image != null) {
				try {
					this.core.getInsnMemory().load(image);
				} catch (IOException e) {
					JOptionPane.showMessageDialog(this, e.getMessage());
					e.printStackTrace();
				}
			}
		});
		toolsPanel.add(loadBtn);
		JButton reloadBtn = new JButton("Reload");
		reloadBtn.addActionListener((ae) -> {
			if (this.prevProgImage != null) {
				try {
					this.core.getInsnMemory().load(this.prevProgImage);
					this.core.reset();
					this.running = false;
					this.updateStatus();
					this.reload();
				} catch (IOException e) {
					JOptionPane.showMessageDialog(this, e.getMessage());
					e.printStackTrace();
				}
			}
		});
		toolsPanel.add(reloadBtn);
		JButton stepBtn = new JButton("Step");
		stepBtn.addActionListener((ae) -> {
			this.resumingFromBreakPoint = true;
			this.tick();
		});
		toolsPanel.add(stepBtn);
		JButton runStopBtn = new JButton("Run / Stop");
		runStopBtn.addActionListener((ae) -> {
			if (this.running ^= true) {
				this.resumingFromBreakPoint = true;
				this.restartRunning();
			}
			
			this.updateStatus();
		});
		toolsPanel.add(runStopBtn);
		JButton resetBtn = new JButton("Reset");
		resetBtn.addActionListener((ae) -> {
			this.core.reset();
			this.running = false;
			this.updateStatus();
			this.reload();
		});
		toolsPanel.add(resetBtn);
		JButton fasterBtn = new JButton("Faster");
		fasterBtn.addActionListener((ae) -> {
			this.tickPeriod = Math.max(1, this.tickPeriod / 2);
			this.restartRunning();
		});
		toolsPanel.add(fasterBtn);
		JButton slowerBtn = new JButton("Slower");
		slowerBtn.addActionListener((ae) -> {
			this.tickPeriod = Math.min(65536, this.tickPeriod * 2);
			this.restartRunning();
		});
		toolsPanel.add(slowerBtn);
		JButton superHotBtn = new JButton("SuperHot");
		superHotBtn.addActionListener((ae) -> {
			this.superHot ^= true;
		});
		toolsPanel.add(superHotBtn);
		toolsPanel.add(this.statusLabel = new JLabel());
		
		// Resize
		int width = this.getWidth() - 40;
		int height = this.getHeight() - 40;
		this.insnScrollPane.setPreferredSize(new Dimension((int) (width * 0.4), (int) (height * 0.9)));
		this.ramScrollPane.setPreferredSize(new Dimension((int) (width * 0.3), (int) (height * 0.45)));
		this.registerScrollPane.setPreferredSize(new Dimension((int) (width * 0.15), (int) (height * 0.45)));
		this.stackScrollPane.setPreferredSize(new Dimension((int) (width * 0.15), (int) (height * 0.45)));
		this.csrScrollPane.setPreferredSize(new Dimension((int) (width * 0.3), (int) (height * 0.45)));
		this.display.setPreferredSize(new Dimension((int) (width * 0.3), (int) (height * 0.45)));
		
		// Setup
		this.insnTable.setAutoscrolls(true);
		this.ramTable.setAutoscrolls(true);
		this.ipsTimer.schedule(new TimerTask() {
					@Override
					public void run() {
						MainFrame.this.updateStatus();
					}
				}, 0, 1000);
	}
	
	private void restartRunning() {
		if (this.currentTickTask != null) {
			this.currentTickTask.cancel();
			this.tickTimer.purge();
		}
		
		this.currentTickTask = this.createTickTask();
		this.tickTimer.schedule(this.currentTickTask, 0, this.tickPeriod);
		this.updateStatus();
	}

	private TimerTask createTickTask() {
		return new TimerTask() {
			@Override
			public void run() {
				if (MainFrame.this.running) {
					MainFrame.this.tick();
				} else {
					this.cancel();
				}
			}
		};
	}

	private void tick() {
		if (EventQueue.isDispatchThread()) {
			if (this.detectBreakpoint()) {
				return;
			}
			
			try {
				this.core.tick();
				this.ticksSinceLastSecond++;
			} catch (CoreHwException e) {
				JOptionPane.showMessageDialog(this, e.getMessage());
				this.running = false;
				e.printStackTrace();
			}
			
			if (!this.superHot) {
				this.reload();
			}
		} else {
			try {
				SwingUtilities.invokeAndWait(() -> {
					this.tick();
				});
			} catch (InvocationTargetException | InterruptedException e) {
				e.printStackTrace();
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

	void reload() {
		int pc = this.core.getPC();
		this.insnTable.reload(this.core.getInsnMemory(), this.breakPoints);
		this.regTable.reload(this.core.getRegFile(), pc);
		this.stackTable.reload(this.core.getCallStack(), this.core.getOperandStack());
		this.csrTable.reload(this.core.getPeriphals());
		this.ramTable.reload(this.core.getDataMemory());
		this.display.reload(this.core.getDataMemory());
		this.insnTable.setRowSelectionInterval(pc, pc);
		this.insnTable.scrollRectToVisible(this.insnTable.getCellRect(pc, 0, true));
		this.repaint();
	}
	
	void updateStatus() {
		String statusLine = String.format("%s, %d IPS, or %.02f MSPI (targeted %d)", 
				this.running ? "Running" : "Stopped", 
				this.ticksSinceLastSecond, 
				1000F / this.ticksSinceLastSecond, 
				this.tickPeriod);
		this.statusLabel.setText(statusLine);
		this.ticksSinceLastSecond = 0;
	}
}
