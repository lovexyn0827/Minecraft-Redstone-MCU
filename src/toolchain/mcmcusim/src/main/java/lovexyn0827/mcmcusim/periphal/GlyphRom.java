package lovexyn0827.mcmcusim.periphal;

import java.awt.Color;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

import javax.imageio.ImageIO;

import it.unimi.dsi.fastutil.ints.IntSet;

public class GlyphRom implements Periphal {
	private static final int GLYADDR_CSR_NO = 0x000;
	private static final int GLYOUT1_CSR_NO = 0x001;
	private static final int GLYOUT2_CSR_NO = 0x002;
	private static final int GLYOUT3_CSR_NO = 0x003;
	private static final IntSet CSR_NUMS = Periphal.buildCsrNumberSet(
			GLYADDR_CSR_NO, GLYOUT1_CSR_NO, GLYOUT2_CSR_NO, GLYOUT3_CSR_NO);
	
	private int glyAddr;
	private int glyOut1;
	private int glyOut2;
	private int glyOut3;
	
	private final int[] glyphs;
	
	public GlyphRom(String atlasFile, int width, int height) {
		this.glyphs = new int[256];
		try {
			BufferedImage atlas = ImageIO.read(new File(atlasFile));
			int glyphsPerRow = atlas.getWidth() / width;
			int rowCount = atlas.getHeight() / height;
			for (int i = 0; i < rowCount; i++) {
				int y = height * i;
				for (int j = 0; j < glyphsPerRow; j++) {
					int x = width * j;
					int ascii = i * glyphsPerRow + j;
					this.glyphs[ascii] = readGlphy(atlas, x, y, width, height);
				}
			}
		} catch (IOException e) {
			System.err.printf("Failed to load glyph atlas!");
			e.printStackTrace();
		}
	}

	private int readGlphy(BufferedImage atlas, int x, int y, int width, int height) {
		int glyph = 0;
		for (int dx = 0; dx < width; dx++) {
			for (int dy = 0; dy < height; dy++) {
				int rgb = atlas.getRGB(x + dx, y + dy);
				int pixel = rgb == -1 ? 0 : 1;
				glyph |=  pixel << (height * dx + dy);
			}
		}
		
		return glyph;
	}

	@Override
	public void tick() {
		int glyph = this.glyphs[this.glyAddr];
		// 3 * 6
		this.glyOut1 = (glyph >> 0) & 0x3F;
		this.glyOut2 = (glyph >> 6) & 0x3F;
		this.glyOut3 = (glyph >> 12) & 0x3F;
	}

	@Override
	public void reset() {
		this.glyAddr = this.glyOut1 = this.glyOut2 = this.glyOut3 = 0;
	}

	@Override
	public int readCsr(int csr) {
		switch (csr) {
		case GLYADDR_CSR_NO:
			return this.glyAddr;
		case GLYOUT1_CSR_NO:
			return this.glyOut1;
		case GLYOUT2_CSR_NO:
			return this.glyOut2;
		case GLYOUT3_CSR_NO:
			return this.glyOut3;
		default:
			throw new AssertionError();
		}
	}

	@Override
	public void writeCsr(int csr, int val) {
		assert (val & (-1 << 8)) == 0;
		switch (csr) {
		case GLYADDR_CSR_NO:
			this.glyAddr = (byte) val;
		}
	}

	@Override
	public IntSet getCsrNumbers() {
		return CSR_NUMS;
	}

}
