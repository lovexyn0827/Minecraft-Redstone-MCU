package lovexyn0827.mcmcusim.gui;

import java.awt.Font;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.JOptionPane;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableModel;

import lovexyn0827.mcmcusim.core.DataMemory;

class SramTable extends JTable {
	private static final long serialVersionUID = -296801970525873393L;
	
	SramTable(VirtualDisplay display) {
		super();
		this.setFont(Font.decode(Font.MONOSPACED));
		
		this.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseClicked(MouseEvent event) {
				if (event.getClickCount() != 2) {
					return;
				}
				
				TableModel tableModel = SramTable.this.getModel();
				if (!(tableModel instanceof Model)) {
					return;
				}
				
				Model model = (Model) tableModel;
				int row = SramTable.this.rowAtPoint(event.getPoint());
				int col = SramTable.this.columnAtPoint(event.getPoint());
				if (row >= 0 && col >= 0) {
					// Exclude PC
					int address = row * 16 + col;
					String newValStr = JOptionPane.showInputDialog(SramTable.this, 
							String.format("New value for byte @ 0x%02x", address));
					try {
						int newVal = Integer.parseInt(newValStr, 16);
						if ((newVal & (-1 << 8)) != 0) {
							JOptionPane.showMessageDialog(SramTable.this, 
									"Input must be a 8-bit heximal number!");
							return;
						}
						
						model.dataMem.write(address, newVal);
					} catch (NumberFormatException e) {
						JOptionPane.showMessageDialog(SramTable.this, 
								"Input must be a 8-bit heximal number!");
						return;
					}
				}
				
				SramTable.this.repaint();
				display.repaint();
			}
		});
	}

	public void reload(DataMemory dataMem) {
		this.setModel(new Model(dataMem));
	}

	class Model extends AbstractTableModel {
		private static final long serialVersionUID = 484774372289788814L;
		private final DataMemory dataMem;
		
		Model(DataMemory dataMem) {
			this.dataMem = dataMem;
		}
		
		@Override
		public int getColumnCount() {
			return 16;
		}

		@Override
		public int getRowCount() {
			return 16;
		}

		@Override
		public Object getValueAt(int row, int col) {
			return String.format("%02x", this.dataMem.read(row * 16 + col));
		}
		
		@Override
		public String getColumnName(int column) {
			return String.format("%X", column);
		}
	}
}
