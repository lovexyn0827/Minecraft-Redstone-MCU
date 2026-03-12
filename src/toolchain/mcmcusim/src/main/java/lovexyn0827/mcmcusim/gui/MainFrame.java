package lovexyn0827.mcmcusim.gui;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.io.File;
import java.util.Timer;
import java.util.TimerTask;

import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import lovexyn0827.mcmcusim.core.Core;
import lovexyn0827.mcmcusim.sim.Simulator;

public class MainFrame extends JFrame {
	private static final long serialVersionUID = 3621344886019049445L;
	private final Simulator simulator;
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
	private Timer statusUpdateTimer = new Timer();
	private JLabel statusLabel;
	
	public MainFrame(String fileName) {
		super();
		this.simulator = new Simulator((sim) -> this.reloadGui(sim), 
				(sim, msg) -> JOptionPane.showMessageDialog(this, msg));
		if (fileName != null) {
			this.simulator.loadProgram(new File(fileName));
		}

		this.init();
		this.setVisible(true);
		this.reloadGui(this.simulator);
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
				this.simulator.loadProgram(image);
			}
		});
		toolsPanel.add(loadBtn);
		JButton reloadBtn = new JButton("Reload");
		reloadBtn.addActionListener((ae) -> {
			this.simulator.reloadProgram();
			this.updateStatus();
		});
		toolsPanel.add(reloadBtn);
		JButton stepBtn = new JButton("Step");
		stepBtn.addActionListener((ae) -> {
			this.simulator.step();
		});
		toolsPanel.add(stepBtn);
		JButton runStopBtn = new JButton("Run / Stop");
		runStopBtn.addActionListener((ae) -> {
			this.simulator.setRunning(!this.simulator.isRunning());
			this.updateStatus();
		});
		toolsPanel.add(runStopBtn);
		JButton resetBtn = new JButton("Reset");
		resetBtn.addActionListener((ae) -> {
			this.simulator.reset();
			this.updateStatus();
		});
		toolsPanel.add(resetBtn);
		JButton fasterBtn = new JButton("Faster");
		fasterBtn.addActionListener((ae) -> {
			int newTickPeriod = Math.max(1, this.simulator.getTickPeriod() / 2);
			this.simulator.setTickPeriod(newTickPeriod);
		});
		toolsPanel.add(fasterBtn);
		JButton slowerBtn = new JButton("Slower");
		slowerBtn.addActionListener((ae) -> {
			int newTickPeriod = Math.min(65536, this.simulator.getTickPeriod() * 2);
			this.simulator.setTickPeriod(newTickPeriod);
		});
		toolsPanel.add(slowerBtn);
		JButton superHotBtn = new JButton("SuperHot");
		superHotBtn.addActionListener((ae) -> {
			this.simulator.setSuperHot(!this.simulator.isSuperHot());
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
		this.statusUpdateTimer.schedule(new TimerTask() {
					@Override
					public void run() {
						MainFrame.this.updateStatus();
					}
				}, 0, 1000);
	}

	void reloadGui(Simulator simulator) {
		Core core = simulator.getCore();
		int pc = core.getPC();
		this.insnTable.reload(core.getInsnMemory(), simulator.getBreakPoints());
		this.regTable.reload(core.getRegFile(), pc);
		this.stackTable.reload(core.getCallStack(), core.getOperandStack());
		this.csrTable.reload(core.getPeriphals());
		this.ramTable.reload(core.getDataMemory());
		this.display.reload(core.getDataMemory());
		this.insnTable.setRowSelectionInterval(pc, pc);
		this.insnTable.scrollRectToVisible(this.insnTable.getCellRect(pc, 0, true));
		this.repaint();
	}
	
	void updateStatus() {
		String statusLine = String.format("%s, %d IPS, or %.02f MSPI (targeted %d)", 
				this.simulator.isRunning() ? "Running" : "Stopped", 
				this.simulator.getIps(), 
				1000F / this.simulator.getIps(),  
				this.simulator.getTickPeriod());
		this.statusLabel.setText(statusLine);
	}
}
