#include "PPU.h"

#include "SupaNES.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <stdio.h>
#include <sys/types.h>

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

	if (HCounter == 339) {
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
		assert((value & 0x80) == 0);
	}
	else if (reg == 0x04) {		// OAMDATA
		if (!OAMFlipflop) {
			OAM[OAMAddress] = value;
			OAMFlipflop = true;
		} else {
			OAM[OAMAddress] |= value << 8;
			OAMAddress++;
			OAMFlipflop = false;
		}
	}
	else if (reg == 0x05) {		// BGMODE
		BGMode = value;
	}
	else if (reg == 0x06) {
		printf("\033[0;32mWritting MOSAIC: %02x\033[0m\n", value);
		assert(value == 0);
	}
	else if (reg == 0x07) {		// BG1SC
		BG1TileAddress = value;
		printf("\033[0;32mWritting BG1SC: %02x\033[0m\n", value);
		assert((BG1TileAddress & 0x3) == 0);
	}
	else if (reg == 0x08) {		// BG2SC
		BG2TileAddress = value;
		printf("\033[0;32mWritting BG2SC: %02x\033[0m\n", value);
		assert((BG2TileAddress & 0x3) == 0);
	}
	else if (reg == 0x09) {		// BG3SC
		BG3TileAddress = value;
		printf("\033[0;32mWritting BG3SC: %02x\033[0m\n", value);
		assert((BG3TileAddress & 0x3) == 0);
	}
	else if (reg == 0x0A) {		// BG4SC
		BG4TileAddress = value;
		printf("\033[0;32mWritting BG4SC: %02x\033[0m\n", value);
		assert((BG4TileAddress & 0x3) == 0);
	}
	else if (reg == 0x0B) {		// BG12NBA
		BGAddress1_2 = value;
		printf("\033[0;32mWritting BG12NBA: %02x\033[0m\n", value);
	}
	else if (reg == 0x0C) {		// BG34NBA
		BGAddress3_4 = value;
		printf("\033[0;32mWritting BG34NBA: %02x\033[0m\n", value);
	}
	else if (reg == 0x0D) {
		printf("\033[0;32mWritting BG1HOFS: %02x\033[0m\n", value);
		assert(value == 0);
	}
	else if (reg == 0x0E) {
		printf("\033[0;32mWritting BG1VOFS: %02x\033[0m\n", value);
		assert(value == 0);
	}
	else if (reg == 0x0F) {
		printf("\033[0;32mWritting BG2HOFS: %02x\033[0m\n", value);
		assert(value == 0);
	}
	else if (reg == 0x10) {
		printf("\033[0;32mWritting BG2VOFS: %02x\033[0m\n", value);
		assert(value == 0);
	}
	else if (reg == 0x11) {
		printf("\033[0;32mWritting BG3HOFS: %02x\033[0m\n", value);
		assert(value == 0);
	}
	else if (reg == 0x12) {
		printf("\033[0;32mWritting BG3VOFS: %02x\033[0m\n", value);
		assert(value == 0);
	}
	else if (reg == 0x13) {
		printf("\033[0;32mWritting BG4HOFS: %02x\033[0m\n", value);
		assert(value == 0);
	}
	else if (reg == 0x14) {
		printf("\033[0;32mWritting BG4VOFS: %02x\033[0m\n", value);
		assert(value == 0);
	}
	else if (reg == 0x15) {		// VMAIN
		VMain = value;
		//printf("\033[0;32mWritting VMAIN: %02x\033[0m\n", value);
		assert((VMain & 0x3) == 0);
		assert((VMain & 0xC) == 0);
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
		VRAM[VRAMAddress] &= 0xFF00;
		VRAM[VRAMAddress] |= value;
		if (!(VMain & 0x80)) {
			VRAMAddress++;
		}
	}
	else if (reg == 0x19) {		// VMDATAH
		VRAM[VRAMAddress] &= 0x00FF;
		VRAM[VRAMAddress] |= value << 8;
		if (VMain & 0x80) {
			VRAMAddress++;
		}
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
		SetBGPixel(X, Y, BG4TileAddress, 4, GetColorBPPForBG(4));
	}
	if (TM & 0x04) {
		SetBGPixel(X, Y, BG3TileAddress, 3, GetColorBPPForBG(3));
	}
	if (TM & 0x02) {
		SetBGPixel(X, Y, BG2TileAddress, 2, GetColorBPPForBG(2));
	}
	if (TM & 0x01) {
		SetBGPixel(X, Y, BG1TileAddress, 1, GetColorBPPForBG(1));
	}
}

void PPU::SetBGPixel(int X, int Y, uint8_t BGAddress, uint8_t BGIndex, uint8_t bpp) {
	uint16_t tile = (((BGAddress & 0xFC) << 8) % 0x8000)	// Mod 0x8000 because we use 16bits values instead of 8bits
					+ ((Y / 8) << 5)
					+ (X / 8);
	uint16_t tileWord = VRAM[tile];

	uint16_t tileID = tileWord & 0x3FF;
	uint8_t BGPaletteNumber = (tileWord >> 10) & 0x7;
	bool BGPriority = tileWord & 0x2000;
	bool BGFlipX = tileWord & 0x4000;
	bool BGFlipY = tileWord & 0x8000;

	uint8_t i = Y % 8;
	uint8_t j = X % 8;
	uint8_t VShift = i;
	uint8_t HShift = 7 - j;

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

	pixels[(Y * width) + X] = palette | 0x8000;
}

void PPU::SetBGDrop(int X, int Y, bool forceBlank) {
	if (forceBlank) {
		pixels[(Y * width) + X] = 0x0000;
	} else {
		pixels[(Y * width) + X] = CGRAM[0] | 0x8000;
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
		return CGRAM[0] | 0x8000;
	}
	return CGRAM[paletteIndex + palette * 4 + (BGIndex - 1) * 32];
}

uint16_t PPU::GetColor4BPP(uint16_t charAddress, uint8_t palette, uint8_t HShift) {
	uint8_t BGLo1 = VRAM[charAddress] & 0xFF;
	uint8_t BGLo2 = VRAM[charAddress] >> 8;
	uint8_t BGHi1 = VRAM[charAddress + 8] & 0xFF;
	uint8_t BGHi2 = VRAM[charAddress + 8] >> 8;
	uint8_t paletteIndex = ((BGLo1 >> HShift) & 1) + (2 * ((BGLo2 >> HShift) & 1))
						+ (4 * ((BGHi1 >> HShift) & 1)) + (8 * ((BGHi2 >> HShift) & 1));

	if (paletteIndex == 0) {
		return CGRAM[0] | 0x8000;
	}
	return CGRAM[paletteIndex + palette * 16];
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
		return CGRAM[0] | 0x8000;
	}
	return CGRAM[paletteIndex];
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
			outPixels[y * (pitch / 2) + x] = pixels[y * (pitch / 2) + x];
		}
	}
}
