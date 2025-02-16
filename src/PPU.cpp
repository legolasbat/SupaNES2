#include "PPU.h"

#include "SupaNES.h"

#include <cassert>
#include <cstdint>
#include <stdio.h>
#include <sys/types.h>

PPU::PPU(SupaNES* supaNes) {
	memory = supaNes;

	HCounter = 46;
	VCounter = 0;
}

void PPU::Clock() {
	if (HCounter < width && VCounter < height) {
		SetPixel(HCounter, VCounter);
	}
	HCounter++;

	if (HCounter >= 339) {
		HCounter -= 339;
		VCounter++;
		if (VCounter == 225) {
			// TODO: CPU NMI
		}
		else if (VCounter == 262) {		// Frame Complete
			frameCompleted = true;
			VCounter = 0;
		}
	}
}

void PPU::WritePPU(uint32_t address, uint8_t value) {
	uint8_t reg = address & 0xFF;

	if (reg == 0x00) {
		printf("\033[0;32mWritting INIDISP: %02x\033[0m\n", value);
	}
	else if (reg == 0x01) {
		printf("\033[0;32mWritting OBSEL: %02x\033[0m\n", value);
	}
	else if (reg == 0x02) {		// OAMADDL
		OAMAddress &= 0xFF00;
		OAMAddress |= value;
	}
	else if (reg == 0x03) {		// OAMADDH
		OAMAddress &= 0xFF00;
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
	else if (reg == 0x05) {
		BGMode = value;
		printf("\033[0;32mWritting BGMODE: %02x\033[0m\n", value);
		assert((BGMode & 0x07) == 0);
	}
	else if (reg == 0x06) {
		printf("\033[0;32mWritting MOSAIC: %02x\033[0m\n", value);
		assert(value == 0);
	}
	else if (reg == 0x07) {
		BG1TileAddress = value;
		printf("\033[0;32mWritting BG1SC: %02x\033[0m\n", value);
		assert((BG1TileAddress & 0x3) == 0);
	}
	else if (reg == 0x08) {
		printf("\033[0;32mWritting BG2SC: %02x\033[0m\n", value);
		assert(value == 0);
	}
	else if (reg == 0x09) {
		printf("\033[0;32mWritting BG3SC: %02x\033[0m\n", value);
		assert(value == 0);
	}
	else if (reg == 0x0A) {
		printf("\033[0;32mWritting BG4SC: %02x\033[0m\n", value);
		assert(value == 0);
	}
	else if (reg == 0x0B) {
		BGAddress1_2 = value;
		printf("\033[0;32mWritting BG12NBA: %02x\033[0m\n", value);
	}
	else if (reg == 0x0C) {
		printf("\033[0;32mWritting BG34NBA: %02x\033[0m\n", value);
		assert(value == 0);
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
		printf("\033[0;32mWritting VMAIN: %02x\033[0m\n", value);
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
	else if (reg == 0x21) {
		printf("\033[0;32mWritting CGADD: %02x\033[0m\n", value);
	}
	else if (reg == 0x22) {
		printf("\033[0;32mWritting CGDATA: %02x\033[0m\n", value);
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
	uint16_t tile = ((BG1TileAddress & 0xFC) << 8)
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

	uint16_t tileOffset = (BGAddress1_2 & 0xF) << 12;

	uint16_t palette = 0;

	pixels[(Y * width) + X] = palette;
}

uint16_t* PPU::GetFrame() {
	frameCompleted = false;
	for (int i = 0; i < width * height; i++) {
		pixels[i] = 0xF0;
	}
	return pixels;
}
