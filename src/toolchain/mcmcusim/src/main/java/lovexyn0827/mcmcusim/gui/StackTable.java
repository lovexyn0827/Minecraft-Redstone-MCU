package lovexyn0827.mcmcusim.gui;

import java.awt.Font;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.JOptionPane;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableModel;

import lovexyn0827.mcmcusim.core.Stack;

class StackTable extends JTable {
	private static final long serialVersionUID = -5373245781292424668L;

	StackTable() {
		super();
		this.setFont(Font.decode(Font.MONOSPACED));
		
		this.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseClicked(MouseEvent event) {
				if (event.getClickCount() != 2) {
					return;
				}
				
				TableModel tableModel = StackTable.this.getModel();
				if (!(tableModel instanceof Model)) {
					return;
				}
				
				Model model = (Model) tableModel;
				int row = StackTable.this.rowAtPoint(event.getPoint());
				int col = StackTable.this.columnAtPoint(event.getPoint());
				if (row > 0 && col >= 0) {
					// Exclude PC
					String newValStr = JOptionPane.showInputDialog(StackTable.this, 
							String.format("New value for r%d", row));
					int wordLen = col == 1 ? 8 : 12;
					try {
						int newVal = Integer.parseInt(newValStr, 16);
						if ((newVal & (-1 << wordLen)) != 0) {
							JOptionPane.showMessageDialog(StackTable.this, String.format(
									"Input must be a %d-bit heximal number!", wordLen));
							return;
						}
						
						if (col == 1) {
							model.callStack.debugSet(row, newVal);
						} else if (col == 2) {
							model.operandStack.debugSet(row, newVal);
						}
					} catch (NumberFormatException e) {
						JOptionPane.showMessageDialog(StackTable.this, String.format(
								"Input must be a %d-bit heximal number!", wordLen));
						return;
					}
				}
				
				StackTable.this.repaint();
			}
		});
	}

	public void reload(Stack callStack, Stack operandStack) {
		this.setModel(new Model(callStack, operandStack));
	}

	class Model extends AbstractTableModel {
		private static final long serialVersionUID = -199874179009503702L;
		private static final String[] COL_NAMES = new String[] { "Row", "Call", "Opnd" };
		private final Stack callStack;
		private final Stack operandStack;
		
		public Model(Stack callStack, Stack operandStack) {
			this.callStack = callStack;
			this.operandStack = operandStack;
		}

		@Override
		public int getColumnCount() {
			return 3;
		}

		@Override
		public int getRowCount() {
			return Math.max(this.callStack.getCapacity(), this.operandStack.getCapacity());
		}

		@Override
		public Object getValueAt(int row, int col) {
			switch (col) {
			case 0:
				return String.format("%d", row);
			case 1:
				if (this.callStack.debugGetPointer() == row) {
					return String.format("-> 0x%03x", this.callStack.debugGet(row));
				} else {
					return String.format("0x%03x", this.callStack.debugGet(row));
				}
			case 2:

				if (this.operandStack.debugGetPointer() == row) {
					return String.format("-> 0x%02x", this.operandStack.debugGet(row));
				} else {
					return String.format("0x%02x", this.operandStack.debugGet(row));
				}
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
