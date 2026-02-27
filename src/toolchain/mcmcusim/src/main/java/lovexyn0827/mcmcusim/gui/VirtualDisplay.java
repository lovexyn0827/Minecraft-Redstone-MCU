package lovexyn0827.mcmcusim.gui;

import java.awt.Canvas;
import java.awt.Color;
import java.awt.Graphics;

import lovexyn0827.mcmcusim.core.DataMemory;

class VirtualDisplay extends Canvas {
	private static final long serialVersionUID = 4133582297742795948L;
	private DataMemory dataMemory;

	public void reload(DataMemory dataMemory) {
		this.dataMemory = dataMemory;
		this.repaint();
	}
	
	@Override
	public void repaint() {
		Graphics g = this.getGraphics();
		if (g == null) {
			return;
		}
		
		g.setColor(Color.BLACK);
		int pixelSize = Math.min(this.getWidth() / 24, this.getHeight() / 16);
		g.fillRect(0, 0, pixelSize * 24, pixelSize * 16);
		g.setColor(Color.WHITE);
		for (int i = 208; i < 256; i++) {
			int word = this.dataMemory.read(i);
			for (int j = 0; j < 8; j++) {
				if ((word & (1 << j)) != 0) {
					// Column-major
					int x = (i - 208) / 2;
					int y = (i * 8 + j) % 16;
					g.fillRect(x * pixelSize, y * pixelSize, pixelSize, pixelSize);
				}
			}
		}
	}

}
