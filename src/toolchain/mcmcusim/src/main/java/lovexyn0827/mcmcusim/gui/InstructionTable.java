package lovexyn0827.mcmcusim.gui;

import java.awt.Font;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.JOptionPane;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableModel;

import lovexyn0827.mcmcusim.core.InstructionMemory;
import lovexyn0827.mcmcusim.core.insn.MalformedInstructionException;

class InstructionTable extends JTable {
	private static final String[] COL_NAMES = new String[] { "BP", "Address", "Binary", "Decompilation" };
	private static final long serialVersionUID = 7306996703965306257L;
	
	InstructionTable() {
		super();
		this.setFont(Font.decode(Font.MONOSPACED));
		this.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseClicked(MouseEvent event) {
				if (event.getClickCount() != 2) {
					return;
				}
				
				TableModel tableModel = InstructionTable.this.getModel();
				if (!(tableModel instanceof Model)) {
					return;
				}
				
				Model model = (Model) tableModel;
				int row = InstructionTable.this.rowAtPoint(event.getPoint());
				int col = InstructionTable.this.columnAtPoint(event.getPoint());
				if (row >= 0 && row < 4096) {
					if (col == 0) {
						model.breakPoints[row] ^= true;
					} else {
						String newValStr = JOptionPane.showInputDialog(InstructionTable.this, 
								String.format("New value for r%d", row));
						try {
							int newVal = Integer.parseInt(newValStr, 16);
							if ((newVal & (-1 << 20)) != 0) {
								JOptionPane.showMessageDialog(InstructionTable.this, 
										"Input must be a 20-bit heximal number!");
								return;
							}
							
							try {
								model.insnMem.setInstruction(row, newVal);
							} catch (MalformedInstructionException e) {
								JOptionPane.showMessageDialog(InstructionTable.this, 
										"Malformed Instruction: " + e.getMessage());
								e.printStackTrace();
							}
						} catch (NumberFormatException e) {
							JOptionPane.showMessageDialog(InstructionTable.this, 
									"Input must be a 20-bit heximal number!");
							return;
						}
					}
				}
				
				InstructionTable.this.repaint();
			}
		});
	}

	public void reload(InstructionMemory insnMem, boolean[] breakPoints) {
		this.setModel(new Model(insnMem, breakPoints));
		this.columnModel.getColumn(0).setPreferredWidth(24);
		this.columnModel.getColumn(1).setPreferredWidth(64);
		this.columnModel.getColumn(2).setPreferredWidth(64);
		this.columnModel.getColumn(3).setPreferredWidth(this.getWidth() - 144);
	}

	class Model extends AbstractTableModel {
		private static final long serialVersionUID = 484774372289788814L;
		private final InstructionMemory insnMem;
		private final boolean[] breakPoints;
		
		Model(InstructionMemory insnMem, boolean[] breakPoints) {
			this.insnMem = insnMem;
			this.breakPoints = breakPoints;
		}
		
		@Override
		public int getColumnCount() {
			return 4;
		}

		@Override
		public int getRowCount() {
			return 4096;
		}

		@Override
		public Object getValueAt(int row, int col) {
			switch (col) {
			case 0:
				return this.breakPoints[row] ? "*" : "";
			case 1:
				return String.format("0x%03x", row);
			case 2:
				return String.format("0x%05x", this.insnMem.fetchInstruction(row).getBinary());
			case 3:
				return this.insnMem.fetchInstruction(row).deassemble();
			default:
				throw new AssertionError();
			}
		}
		
		@Override
		public String getColumnName(int column) {
			return COL_NAMES[column];
		}
	}
}
