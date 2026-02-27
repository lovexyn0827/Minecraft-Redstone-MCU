package lovexyn0827.mcmcusim.gui;

import java.awt.Font;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.JOptionPane;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableModel;

import it.unimi.dsi.fastutil.ints.Int2ObjectMap;
import lovexyn0827.mcmcusim.periphal.Periphal;

class CSRTable extends JTable {
	private static final long serialVersionUID = -5604471747210443587L;

	CSRTable() {
		super();
		this.setFont(Font.decode(Font.MONOSPACED));
		
		this.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseClicked(MouseEvent event) {
				if (event.getClickCount() != 2) {
					return;
				}
				
				TableModel tableModel = CSRTable.this.getModel();
				if (!(tableModel instanceof Model)) {
					return;
				}
				
				Model model = (Model) tableModel;
				int row = CSRTable.this.rowAtPoint(event.getPoint());
				int col = CSRTable.this.columnAtPoint(event.getPoint());
				if (row >= 0 && col >= 0) {
					// Exclude PC
					int csrNo = row * 16 + col;
					String newValStr = JOptionPane.showInputDialog(CSRTable.this, 
							String.format("New value for CSR 0x%03x", csrNo));
					try {
						int newVal = Integer.parseInt(newValStr, 16);
						if ((newVal & (-1 << 10)) != 0) {
							JOptionPane.showMessageDialog(CSRTable.this, 
									"Input must be a 10-bit heximal number!");
							return;
						}
						
						model.periphals.get(csrNo).writeCsr(csrNo, newVal);
					} catch (NumberFormatException e) {
						JOptionPane.showMessageDialog(CSRTable.this, 
								"Input must be a 10-bit heximal number!");
						return;
					}
				}
				
				CSRTable.this.repaint();
			}
		});
	}

	public void reload(Int2ObjectMap<Periphal> periphals) {
		this.setModel(new Model(periphals));
	}

	class Model extends AbstractTableModel {
		private static final long serialVersionUID = 484774372289788814L;
		private final Int2ObjectMap<Periphal> periphals;
		
		Model(Int2ObjectMap<Periphal> periphals) {
			this.periphals = periphals;
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
			int csrNo = row * 16 + col;
			return String.format("%02x", this.periphals.get(csrNo).readCsr(csrNo));
		}
		
		@Override
		public String getColumnName(int column) {
			return String.format("%X", column);
		}
	}
}
