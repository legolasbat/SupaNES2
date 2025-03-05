#include "PPU.h"

#include "SupaNES.h"

#include <cassert>
#include <stdio.h>

PPU::PPU(SupaNES* supaNes) {
	memory = supaNes;

	HCounter = 46;
	VCounter = 0;
	frameCounter = 0;
}

void PPU::Clock() {
	if (HCounter < width && VCounter < height) {
		SetPixel(HCounter, VCounter);
	}
	HCounter++;

	if (HCounter == 256) {
		memory->cpu->StartHDMA();
	}
	else if (HCounter == 339) {
		HCounter = 0;
		VCounter++;
		if (VCounter == 225) {
			memory->cpu->NMI();
			frameCompleted = true;
		}
		else if (VCounter == 262) {
			frameCounter++;
			if (frameCounter == 60) {
				frameCounter = 0;
			}
			VCounter = 0;
			memory->cpu->ResetHDMA();
		}
	}
}

void PPU::WritePPU(uint32_t address, uint8_t value) {
	uint8_t reg = address & 0xFF;

	if (reg == 0x00) {			// INIDISP
		inidisp = value;
	}
	else if (reg == 0x01) {
		printf("\033[0;32mWritting OBSEL: %02x\033[0m\n", value);
	}
	else if (reg == 0x02) {		// OAMADDL
		OAMAddress &= 0xFF00;
		OAMAddress |= value;
	}
	else if (reg == 0x03) {		// OAMADDH
		OAMAddress &= 0x00FF;
		OAMAddress |= value << 8;
		OAMFlipflop = false;
	}
	else if (reg == 0x04) {		// OAMDATA
		if (!OAMFlipflop) {
			OAM[OAMAddress] = value;
			OAMFlipflop = true;
		} else {
			OAM[OAMAddress] |= value << 8;
			OAMAddress++;
			OAMAddress = OAMAddress & 0x110;
			OAMFlipflop = false;
		}
	}
	else if (reg == 0x05) {		// BGMODE
		BGMode = value;
	}
	else if (reg == 0x06) {		// MOSAIC
		mosaic = value;
	}
	else if (reg == 0x07) {		// BG1SC
		BG1TileAddress = value;
	}
	else if (reg == 0x08) {		// BG2SC
		BG2TileAddress = value;
	}
	else if (reg == 0x09) {		// BG3SC
		BG3TileAddress = value;
	}
	else if (reg == 0x0A) {		// BG4SC
		BG4TileAddress = value;
	}
	else if (reg == 0x0B) {		// BG12NBA
		BGAddress1_2 = value;
	}
	else if (reg == 0x0C) {		// BG34NBA
		BGAddress3_4 = value;
	}
	else if (reg == 0x0D) {		// BG1HOFS or M7HOFS
		if (!BG1HScrollFlipflop) {
			BG1HScroll &= 0xFF00;
			BG1HScroll |= value;
			BG1HScrollFlipflop = true;
		} else {
			BG1HScroll &= 0x00FF;
			BG1HScroll |= (value << 8);
			BG1HScrollFlipflop = false;
		}
		printf("\033[0;32mWritting BG1HOFS: %02x\033[0m\n", value);
		assert((BGMode & 0x7) != 7);
	}
	else if (reg == 0x0E) {		// BG1VOFS or M7VOFS
		if (!BG1VScrollFlipflop) {
			BG1VScroll &= 0xFF00;
			BG1VScroll |= value;
			BG1VScrollFlipflop = true;
		} else {
			BG1VScroll &= 0x00FF;
			BG1VScroll |= (value << 8);
			BG1VScrollFlipflop = false;
		}
		printf("\033[0;32mWritting BG1VOFS: %02x\033[0m\n", value);
		assert((BGMode & 0x7) != 7);
	}
	else if (reg == 0x0F) {		// BG2HOFS
		if (!BG2HScrollFlipflop) {
			BG2HScroll &= 0xFF00;
			BG2HScroll |= value;
			BG2HScrollFlipflop = true;
		} else {
			BG2HScroll &= 0x00FF;
			BG2HScroll |= (value << 8);
			BG2HScrollFlipflop = false;
		}
	}
	else if (reg == 0x10) {		// BG2VOFS
		if (!BG2VScrollFlipflop) {
			BG2VScroll &= 0xFF00;
			BG2VScroll |= value;
			BG2VScrollFlipflop = true;
		} else {
			BG2VScroll &= 0x00FF;
			BG2VScroll |= (value << 8);
			BG2VScrollFlipflop = false;
		}
	}
	else if (reg == 0x11) {		// BG3HOFS
		if (!BG3HScrollFlipflop) {
			BG3HScroll &= 0xFF00;
			BG3HScroll |= value;
			BG3HScrollFlipflop = true;
		} else {
			BG3HScroll &= 0x00FF;
			BG3HScroll |= (value << 8);
			BG3HScrollFlipflop = false;
		}
	}
	else if (reg == 0x12) {		// BG3VOFS
		if (!BG3VScrollFlipflop) {
			BG3VScroll &= 0xFF00;
			BG3VScroll |= value;
			BG3VScrollFlipflop = true;
		} else {
			BG3VScroll &= 0x00FF;
			BG3VScroll |= (value << 8);
			BG3VScrollFlipflop = false;
		}
	}
	else if (reg == 0x13) {		// BG4HOFS
		if (!BG4HScrollFlipflop) {
			BG4HScroll &= 0xFF00;
			BG4HScroll |= value;
			BG4HScrollFlipflop = true;
		} else {
			BG4HScroll &= 0x00FF;
			BG4HScroll |= (value << 8);
			BG4HScrollFlipflop = false;
		}
	}
	else if (reg == 0x14) {		// BG4VOFS
		if (!BG4VScrollFlipflop) {
			BG4VScroll &= 0xFF00;
			BG4VScroll |= value;
			BG4VScrollFlipflop = true;
		} else {
			BG4VScroll &= 0x00FF;
			BG4VScroll |= (value << 8);
			BG4VScrollFlipflop = false;
		}
	}
	else if (reg == 0x15) {		// VMAIN
		VMain = value;
	}
	else if (reg == 0x16) {		// VMAADDL
		VRAMAddress &= 0xFF00;
		VRAMAddress |= value;
	}
	else if (reg == 0x17) {		// VMADDH
		VRAMAddress &= 0x00FF;
		VRAMAddress |= value << 8;
	}
	else if (reg == 0x18) {		// VMDATAL
		uint16_t remapAddress = VRAMAddress;
		if ((VMain & 0xC) != 0) {
			if ((VMain & 0xC) == 0x4) {
				uint8_t lower = VRAMAddress & 0xFF;
				remapAddress &= 0xFF00;
				remapAddress |= (lower >> 5) | (lower << 3);
			}
			else if ((VMain & 0xC) == 0x8) {
				uint16_t lower = VRAMAddress & 0x1FF;
				remapAddress &= 0xFE00;
				remapAddress |= (lower >> 6) | (lower << 3);
			}
			else if ((VMain & 0xC) == 0xC) {
				uint16_t lower = VRAMAddress & 0x3FF;
				remapAddress &= 0xFC00;
				remapAddress |= (lower >> 7) | (lower << 3);
			}
		}
		VRAM[remapAddress] &= 0xFF00;
		VRAM[remapAddress] |= value;
		if (!(VMain & 0x80)) {
			if ((VMain & 0x3) == 0) {
				VRAMAddress++;
			}
			else if ((VMain & 0x3) == 1) {
				VRAMAddress += 32;
			}
			else {
				VRAMAddress += 128;
			}
			VRAMAddress = VRAMAddress % 0x8000;
		}
	}
	else if (reg == 0x19) {		// VMDATAH
		uint16_t remapAddress = VRAMAddress;
		if ((VMain & 0xC) != 0) {
			if ((VMain & 0xC) == 0x4) {
				uint8_t lower = VRAMAddress & 0xFF;
				remapAddress &= 0xFF00;
				remapAddress |= (lower >> 5) | (lower << 3);
			}
			else if ((VMain & 0xC) == 0x8) {
				uint16_t lower = VRAMAddress & 0x1FF;
				remapAddress &= 0xFE00;
				remapAddress |= (lower >> 6) | (lower << 3);
			}
			else if ((VMain & 0xC) == 0xC) {
				uint16_t lower = VRAMAddress & 0x3FF;
				remapAddress &= 0xFC00;
				remapAddress |= (lower >> 7) | (lower << 3);
			}
		}
		VRAM[remapAddress] &= 0x00FF;
		VRAM[remapAddress] |= value << 8;
		if (VMain & 0x80) {
			if ((VMain & 0x3) == 0) {
				VRAMAddress++;
			}
			else if ((VMain & 0x3) == 1) {
				VRAMAddress += 32;
			}
			else {
				VRAMAddress += 128;
			}
			VRAMAddress = VRAMAddress % 0x8000;
		}
	}
	else if (reg == 0x1A) {
		printf("\033[0;32mWritting M7SEL: %02x\033[0m\n", value);
	}
	else if (reg == 0x1B) {
		printf("\033[0;32mWritting M7A: %02x\033[0m\n", value);
	}
	else if (reg == 0x1C) {
		printf("\033[0;32mWritting M7B: %02x\033[0m\n", value);
	}
	else if (reg == 0x1D) {
		printf("\033[0;32mWritting M7C: %02x\033[0m\n", value);
	}
	else if (reg == 0x1E) {
		printf("\033[0;32mWritting M7D: %02x\033[0m\n", value);
	}
	else if (reg == 0x1F) {
		printf("\033[0;32mWritting M7X: %02x\033[0m\n", value);
	}
	else if (reg == 0x20) {
		printf("\033[0;32mWritting M7Y: %02x\033[0m\n", value);
	}
	else if (reg == 0x21) {		// CGADD
		CGRAMAddress = value;
		CGRAMFlipflop = false;
	}
	else if (reg == 0x22) {		// CGDATA
		if (!CGRAMFlipflop) {
			CGRAM[CGRAMAddress] = value;
			CGRAMFlipflop = true;
		} else {
			CGRAM[CGRAMAddress] |= value << 8;
			CGRAMAddress++;
			CGRAMAddress = CGRAMAddress % 0x100;
			CGRAMFlipflop = false;
		}
	}
	else if (reg == 0x23) {
		printf("\033[0;32mWritting W12SEL: %02x\033[0m\n", value);
	}
	else if (reg == 0x24) {
		printf("\033[0;32mWritting W34SEL: %02x\033[0m\n", value);
	}
	else if (reg == 0x25) {
		printf("\033[0;32mWritting WOBJSEL: %02x\033[0m\n", value);
	}
	else if (reg == 0x26) {
		printf("\033[0;32mWritting WH0: %02x\033[0m\n", value);
	}
	else if (reg == 0x27) {
		printf("\033[0;32mWritting WH1: %02x\033[0m\n", value);
	}
	else if (reg == 0x28) {
		printf("\033[0;32mWritting WH2: %02x\033[0m\n", value);
	}
	else if (reg == 0x29) {
		printf("\033[0;32mWritting WH3: %02x\033[0m\n", value);
	}
	else if (reg == 0x2A) {
		printf("\033[0;32mWritting WBGLOG: %02x\033[0m\n", value);
	}
	else if (reg == 0x2B) {
		printf("\033[0;32mWritting WOBJLOG: %02x\033[0m\n", value);
	}
	else if (reg == 0x2C) {
		TM = value;
		printf("\033[0;32mWritting TM: %02x\033[0m\n", value);
	}
	else if (reg == 0x2D) {
		printf("\033[0;32mWritting TS: %02x\033[0m\n", value);
	}
	else if (reg == 0x2E) {
		printf("\033[0;32mWritting TMW: %02x\033[0m\n", value);
	}
	else if (reg == 0x2F) {
		printf("\033[0;32mWritting TSW: %02x\033[0m\n", value);
	}
	else if (reg == 0x30) {
		printf("\033[0;32mWritting CGWSEL: %02x\033[0m\n", value);
	}
	else if (reg == 0x31) {
		printf("\033[0;32mWritting CGADSUB: %02x\033[0m\n", value);
	}
	else if (reg == 0x32) {
		printf("\033[0;32mWritting COLDATA: %02x\033[0m\n", value);
	}
	else if (reg == 0x33) {
		printf("\033[0;32mWritting SETINI: %02x\033[0m\n", value);
	}
	else if (reg == 0x34) {
		printf("\033[0;32mWritting MPYL (read only): %02x\033[0m\n", value);
	}
	else {
		printf("%06x: %02x\n", address, value);
		assert(false && "Writting PPU");
	}
}

void PPU::SetPixel(int X, int Y) {
	if (inidisp & 0x80) {
		SetBGDrop(X, Y, true);
		return;
	}
	SetBGDrop(X, Y);
	if (TM & 0x08) {
		SetBGPixel(X, Y, BG4TileAddress, 4, GetColorBPPForBG(4),
				(BG4TileAddress & 0x1) ? 64 : 32, (BG4TileAddress & 0x2) ? 64 : 32,
				BG4HScroll, BG4VScroll);
	}
	if (TM & 0x04) {
		SetBGPixel(X, Y, BG3TileAddress, 3, GetColorBPPForBG(3),
				(BG3TileAddress & 0x1) ? 64 : 32, (BG3TileAddress & 0x2) ? 64 : 32,
				BG3HScroll, BG3VScroll);
	}
	if (TM & 0x02) {
		SetBGPixel(X, Y, BG2TileAddress, 2, GetColorBPPForBG(2),
				(BG2TileAddress & 0x1) ? 64 : 32, (BG2TileAddress & 0x2) ? 64 : 32,
				BG2HScroll, BG2VScroll);
	}
	if (TM & 0x01) {
		SetBGPixel(X, Y, BG1TileAddress, 1, GetColorBPPForBG(1),
				(BG1TileAddress & 0x1) ? 64 : 32, (BG1TileAddress & 0x2) ? 64 : 32,
				BG1HScroll, BG1VScroll);
	}
}

void PPU::SetBGPixel(int X, int Y, uint8_t BGAddress, uint8_t BGIndex, uint8_t bpp,
						uint8_t XSize, uint8_t YSize,
						uint16_t HScroll, uint16_t VScroll) {
	int XScrolled = (X + HScroll) % (8 * XSize);
	int YScrolled = (Y + VScroll) % (8 * YSize);

	uint16_t tile = (((BGAddress & 0xFC) << 8) % 0x8000)	// Mod 0x8000 because we use 16bits values instead of 8bits
					+ ((((XSize == 64) ? (YScrolled % 256) : YScrolled) / 8) << 5)
					+ ((XScrolled % 256) / 8)
					+ (XScrolled / 0x100) * 0x400
					+ (XSize / 64) * ((YScrolled / 0x100) * 0x800);
	uint16_t tileWord = VRAM[tile];

	uint16_t tileID = tileWord & 0x3FF;
	uint8_t BGPaletteNumber = (tileWord >> 10) & 0x7;
	bool BGPriority = tileWord & 0x2000;
	bool BGFlipX = tileWord & 0x4000;
	bool BGFlipY = tileWord & 0x8000;

	uint8_t i = YScrolled % 8;
	uint8_t j = XScrolled % 8;
	uint8_t VShift = i + (-2 * i + 7) * BGFlipY;
	uint8_t HShift = (7 - j) + (2 * j - 7) * BGFlipX;

	uint16_t tileOffset = 0;
	if (BGIndex == 1) {
		tileOffset = (BGAddress1_2 & 0x0F) << 12;
	}
	else if (BGIndex == 2) {
		tileOffset = (BGAddress1_2 & 0xF0) << 8;
	}
	else if (BGIndex == 3) {
		tileOffset = (BGAddress3_4 & 0x0F) << 12;
	}
	else if (BGIndex == 4) {
		tileOffset = (BGAddress3_4 & 0xF0) << 8;
	}

	uint16_t palette = 0x0000;
	uint16_t tileAddress = 0;
	// BPP2
	if (bpp == 2) {
		tileAddress = tileOffset + tileID * 0x8 + VShift;
		palette = GetColor2BPP(tileAddress, BGPaletteNumber, HShift, BGIndex);
	}
	// BPP4
	else if (bpp == 4) {
		tileAddress = tileOffset + tileID * 0x10 + VShift;
		palette = GetColor4BPP(tileAddress, BGPaletteNumber, HShift);
	}
	// BPP8
	else {
		tileAddress = tileOffset + tileID * 0x20 + VShift;
		palette = GetColor8BPP(tileAddress, BGPaletteNumber, HShift);
	}

	if ((palette & 0x8000) == 0 && mosaic == 0) {
		return;
	}

	uint8_t mosaicSize = 0;
	if (mosaic & (1 << (BGIndex - 1)) && (mosaic & 0xF0) >= 0x10) {
		mosaicSize = mosaic >> 4;
	}
	if (mosaicSize >= 1) {
		mosaicSize++;
		if (Y % mosaicSize != 0 || X % mosaicSize != 0) {
			palette = pixels[((Y - (Y % mosaicSize)) * width) + (X - (X % mosaicSize))];
		}
	}

	// Apply brightness
	if ((inidisp & 0xF) != 0xF) {
		float brightness = ((inidisp & 0xF) + 1) / 16.0f;
		uint8_t red = palette & 0x1F;
		uint8_t green = (palette >> 5) & 0x1F;
		uint8_t blue = (palette >> 10) & 0x1F;
		palette &= ~0x1F;
		palette |= (uint8_t)(red * brightness) & 0x1F;
		palette &= ~(0x1F << 5);
		palette |= ((uint8_t)(green * brightness) & 0x1F) << 5;
		palette &= ~(0x1F << 10);
		palette |= ((uint8_t)(blue * brightness) & 0x1F) << 10;
	}

	pixels[(Y * width) + X] = palette;
}

void PPU::SetBGDrop(int X, int Y, bool forceBlank) {
	if (forceBlank) {
		pixels[(Y * width) + X] = 0x0000;
	} else {
		pixels[(Y * width) + X] = CGRAM[0];
	}
}

uint8_t PPU::GetColorBPPForBG(uint8_t BGIndex) {
	uint8_t mode = BGMode & 0x07;

	if (mode == 0) {
		return 2;
	}
	if (mode == 1) {
		if (BGIndex == 3) {
			return 2;
		}
		return 4;
	}
	if (mode == 2) {
		return 4;
	}
	if (mode == 3) {
		if (BGIndex == 1) {
			return 8;
		}
		return 4;
	}
	assert(false && "BG Mode not found.");
	return 0;
}

uint16_t PPU::GetColor2BPP(uint16_t charAddress, uint8_t palette, uint8_t HShift, uint8_t BGIndex) {
	uint8_t BGLo = VRAM[charAddress] & 0xFF;
	uint8_t BGHi = VRAM[charAddress] >> 8;
	uint8_t paletteIndex = ((BGLo >> HShift) & 1) + (2 * ((BGHi >> HShift) & 1));

	if (paletteIndex == 0) {
		return CGRAM[0];
	}
	uint8_t mode = BGMode & 0x07;
	if (mode == 0) {
		return CGRAM[paletteIndex + palette * 4 + (BGIndex - 1) * 32] | 0x8000;
	}
	return CGRAM[paletteIndex + palette * 4] | 0x8000;
}

uint16_t PPU::GetColor4BPP(uint16_t charAddress, uint8_t palette, uint8_t HShift) {
	uint8_t BGLo1 = VRAM[charAddress] & 0xFF;
	uint8_t BGLo2 = VRAM[charAddress] >> 8;
	uint8_t BGHi1 = VRAM[charAddress + 8] & 0xFF;
	uint8_t BGHi2 = VRAM[charAddress + 8] >> 8;
	uint8_t paletteIndex = ((BGLo1 >> HShift) & 1) + (2 * ((BGLo2 >> HShift) & 1))
						+ (4 * ((BGHi1 >> HShift) & 1)) + (8 * ((BGHi2 >> HShift) & 1));

	if (paletteIndex == 0) {
		return CGRAM[0];
	}
	return CGRAM[paletteIndex + palette * 16] | 0x8000;
}

uint16_t PPU::GetColor8BPP(uint16_t charAddress, uint8_t palette, uint8_t HShift) {
	uint8_t BG1 = VRAM[charAddress] & 0xFF;
	uint8_t BG2 = VRAM[charAddress] >> 8;
	uint8_t BG3 = VRAM[charAddress + 8] & 0xFF;
	uint8_t BG4 = VRAM[charAddress + 8] >> 8;
	uint8_t BG5 = VRAM[charAddress + 16] & 0xFF;
	uint8_t BG6 = VRAM[charAddress + 16] >> 8;
	uint8_t BG7 = VRAM[charAddress + 24] & 0xFF;
	uint8_t BG8 = VRAM[charAddress + 24] >> 8;
	uint8_t paletteIndex = ((BG1 >> HShift) & 1) + (2 * ((BG2 >> HShift) & 1))
						+ (4 * ((BG3 >> HShift) & 1)) + (8 * ((BG4 >> HShift) & 1))
						+ (16 * ((BG5 >> HShift) & 1)) + (32 * ((BG6 >> HShift) & 1))
						+ (64 * ((BG7 >> HShift) & 1)) + (128 * ((BG8 >> HShift) & 1));

	if (paletteIndex == 0) {
		return CGRAM[0];
	}
	return CGRAM[paletteIndex] | 0x8000;
}

uint16_t* PPU::GetFrame() {
	frameCompleted = false;
	return pixels;
}

void PPU::SetFrame(void* texturePixels, int pitch) {
	frameCompleted = false;
	uint16_t* outPixels = (uint16_t*)texturePixels;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			outPixels[y * (pitch / 2) + x] = pixels[y * (pitch / 2) + x] | 0x8000;
		}
	}
}
