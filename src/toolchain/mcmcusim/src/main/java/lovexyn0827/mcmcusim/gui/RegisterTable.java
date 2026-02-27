package lovexyn0827.mcmcusim.gui;

import java.awt.Font;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.JOptionPane;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableModel;

import lovexyn0827.mcmcusim.core.RegFile;

class RegisterTable extends JTable {
	private static final long serialVersionUID = -6163368472303401030L;

	RegisterTable() {
		super();
		this.setFont(Font.decode(Font.MONOSPACED));
		
		this.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseClicked(MouseEvent event) {
				if (event.getClickCount() != 2) {
					return;
				}
				
				TableModel tableModel = RegisterTable.this.getModel();
				if (!(tableModel instanceof Model)) {
					return;
				}
				
				Model model = (Model) tableModel;
				int row = RegisterTable.this.rowAtPoint(event.getPoint());
				int col = RegisterTable.this.columnAtPoint(event.getPoint());
				if (row > 0 && col >= 0) {
					// Exclude PC
					String newValStr = JOptionPane.showInputDialog(RegisterTable.this, 
							String.format("New value for r%d", row));
					try {
						int newVal = Integer.parseInt(newValStr, 16);
						if ((newVal & (-1 << 8)) != 0) {
							JOptionPane.showMessageDialog(RegisterTable.this, 
									"Input must be a 8-bit heximal number!");
							return;
						}
						
						model.rf.write(row, newVal);
					} catch (NumberFormatException e) {
						JOptionPane.showMessageDialog(RegisterTable.this, 
								"Input must be a 8-bit heximal number!");
						return;
					}
				}
				
				RegisterTable.this.repaint();
			}
		});
	}

	public void reload(RegFile rf, int pc) {
		this.setModel(new Model(rf, pc));
	}

	class Model extends AbstractTableModel {
		private static final String[] COL_NAMES = new String[] { "Name", "Value" };
		private static final long serialVersionUID = 484774372289788814L;
		private final RegFile rf;
		private final int pc;
		
		public Model(RegFile rf, int pc) {
			this.rf = rf;
			this.pc = pc;
		}

		@Override
		public int getColumnCount() {
			return 2;
		}

		@Override
		public int getRowCount() {
			return 16;
		}

		@Override
		public Object getValueAt(int row, int col) {
			if (col == 0) {
				if (row == 0) {
					return "PC";
				} else {
					return String.format("r%d", row);
				}
			} else {

				if (row == 0) {
					return String.format("0x%03x", this.pc);
				} else {
					return String.format("0x%02x", this.rf.read(row));
				}
			}
		}
		
		@Override
		public String getColumnName(int column) {
			return COL_NAMES[column];
		}
	}
}
