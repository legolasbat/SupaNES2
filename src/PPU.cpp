#include "PPU.h"

#include "SupaNES.h"

#include <cassert>
#include <stdio.h>

PPU::PPU(SupaNES* supaNes) {
	memory = supaNes;

	HCounter = 46;
	VCounter = 0;
	frameCounter = 0;

	OBSEL = 0;
	BGMode = 0;
	fixedColor = 0;

	OAMAddress = 0;

	BG1HScroll = 0;
	M7HOFS = 0;
	BG1VScroll = 0;
	M7VOFS = 0;

	BG2HScroll = 0;
	BG2VScroll = 0;

	BG3HScroll = 0;
	BG3VScroll = 0;

	BG4HScroll = 0;
	BG4VScroll = 0;

	W12SEL = 0;
	W34SEL = 0;
	WOBJSEL = 0;

	window1Left = 0;
	window1Right = 0;
	window2Left = 0;
	window2Right = 0;

	WBGLOG = 0;
	WOBJLOG = 0;

	ObjSizes[0][0] = 8;
	ObjSizes[0][1] = 8;
	ObjSizes[0][2] = 8;
	ObjSizes[0][3] = 16;
	ObjSizes[0][4] = 16;
	ObjSizes[0][5] = 32;
	ObjSizes[0][6] = 0;
	ObjSizes[0][7] = 0;
	ObjSizes[1][0] = 16;
	ObjSizes[1][1] = 32;
	ObjSizes[1][2] = 64;
	ObjSizes[1][3] = 32;
	ObjSizes[1][4] = 64;
	ObjSizes[1][5] = 64;
	ObjSizes[1][6] = 0;
	ObjSizes[1][7] = 0;

	for (int i = 0; i < 0x8000; i++) {
		VRAM[i] = 0;
	}
}

void PPU::Clock() {
	HCounter++;

	// Draw next scanline
	if (HCounter == 256) {
		if (VCounter + 1 < height) {
			for (int i = 0; i < width; i++) {
				SetPixel(i, VCounter + 1);
			}
		}
	}
	// Check HDMA
	else if (HCounter == 0x112) {
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
			isEven = !isEven;
			if (frameCounter == 60) {
				frameCounter = 0;
			}
			VCounter = 0;
			memory->cpu->ResetHDMA();
			memory->cpu->StartHDMA();
			memory->cpu->EndNMI();
			OBJReloadSize();
			GetOBJsToDisplay();
			for (int i = 0; i < width; i++) {
				SetPixel(i, VCounter);
			}
		}
	}
	memory->cpu->IRQ();
}

uint8_t PPU::ReadPPU(uint32_t address) {
	uint8_t value = 0;

	uint8_t reg = address & 0xFF;

	if (reg < 0x34) {
		// Write only
	}
	else if (reg == 0x34) {		// MPYL
		value = multResult & 0xFF;
	}
	else if (reg == 0x35) {		// MPYM
		value = (multResult >> 8) & 0xFF;
	}
	else if (reg == 0x36) {		// MPYH
		value = (multResult >> 16) & 0xFF;
	}
	else if (reg == 0x37) {		// SLHV
		value = 0;	// Actually it should be open bus
		// Reading here updates regs 0x3C and 0x3D
	}
	else if (reg == 0x39) {		// VMDATALREAD
		uint16_t remapAddress = VRAMAddress;
		if ((VMain & 0xC) != 0) {
			if ((VMain & 0xC) == 0x4) {
				uint8_t lower = VRAMAddress & 0xFF;
				remapAddress &= 0xFF00;
				remapAddress |= (lower >> 5) | (lower << 3 & 0x00F8);
			}
			else if ((VMain & 0xC) == 0x8) {
				uint16_t lower = VRAMAddress & 0x1FF;
				remapAddress &= 0xFE00;
				remapAddress |= (lower >> 6) | (lower << 3 & 0x01F8);
			}
			else if ((VMain & 0xC) == 0xC) {
				uint16_t lower = VRAMAddress & 0x3FF;
				remapAddress &= 0xFC00;
				remapAddress |= (lower >> 7) | (lower << 3 & 0x03F8);
			}
		}
		remapAddress %= 0x8000;
		value = VRAM[remapAddress] & 0x00FF;
		if (VRAMAddressReset) {
			VRAMAddressReset = false;
		}
		else if (!(VMain & 0x80)) {
			if ((VMain & 0x3) == 0) {
				VRAMAddress++;
			}
			else if ((VMain & 0x3) == 1) {
				VRAMAddress += 32;
			}
			else {
				VRAMAddress += 128;
			}
		}
	}
	else if (reg == 0x3A) {		// VMDATAHREAD
		uint16_t remapAddress = VRAMAddress;
		if ((VMain & 0xC) != 0) {
			if ((VMain & 0xC) == 0x4) {
				uint8_t lower = VRAMAddress & 0xFF;
				remapAddress &= 0xFF00;
				remapAddress |= (lower >> 5) | (lower << 3 & 0x00F8);
			}
			else if ((VMain & 0xC) == 0x8) {
				uint16_t lower = VRAMAddress & 0x1FF;
				remapAddress &= 0xFE00;
				remapAddress |= (lower >> 6) | (lower << 3 & 0x01F8);
			}
			else if ((VMain & 0xC) == 0xC) {
				uint16_t lower = VRAMAddress & 0x3FF;
				remapAddress &= 0xFC00;
				remapAddress |= (lower >> 7) | (lower << 3 & 0x03F8);
			}
		}
		remapAddress %= 0x8000;
		value = VRAM[remapAddress] >> 8;
		if (VRAMAddressReset) {
			VRAMAddressReset = false;
		}
		else if (VMain & 0x80) {
			if ((VMain & 0x3) == 0) {
				VRAMAddress++;
			}
			else if ((VMain & 0x3) == 1) {
				VRAMAddress += 32;
			}
			else {
				VRAMAddress += 128;
			}
		}
	}
	else if (reg == 0x3B) {		// CGDATA
		if (!CGRAMFlipflop) {
			value = CGRAM[CGRAMAddress];
			CGRAMFlipflop = true;
		}
		else {
			value = CGRAM[CGRAMAddress] >> 8;
			CGRAMAddress++;
			CGRAMAddress = CGRAMAddress % 0x100;
			CGRAMFlipflop = false;
		}
	}
	else if (reg == 0x3C) {		// OPHCT
		if (!OPHCTFlipflop) {
			value = HCounter & 0xFF;
			OPHCTFlipflop = true;
		}
		else {
			value = HCounter >> 8;
			OPHCTFlipflop = false;
		}
	}
	else if (reg == 0x3D) {		// OPVCT
		if (!OPVCTFlipflop) {
			value = VCounter & 0xFF;
			OPVCTFlipflop = true;
		}
		else {
			value = VCounter >> 8;
			OPVCTFlipflop = false;
		}
	}
	else if (reg == 0x3E) {		// STAT77
		printf("Reading PPU STAT77\n");
		value = 0x01;			// 3-0 PPU1 Version
	}
	else if (reg == 0x3F) {		// STAT78
		printf("Reading PPU STAT78\n");
		value = isEven ? 0x80 : 0x00;
		// BIT 4 is 0 if NTSC or 1 if PAL
		value |= 0x01;			// 3-0 PPU2 Version
		OPHCTFlipflop = false;
		OPVCTFlipflop = false;
	}
	else {
		printf("Reading PPU at %06x\n", address);
		assert(false);
	}

	return value;
}

void PPU::WritePPU(uint32_t address, uint8_t value) {
	uint8_t reg = address & 0xFF;

	if (reg == 0x00) {			// INIDISP
		inidisp = value;
	}
	else if (reg == 0x01) {		// OBSEL
		OBSEL = value;
	}
	else if (reg == 0x02) {		// OAMADDL
		OAMAddress &= 0xFF00;
		OAMAddress |= value;
		WrittenInternalOAMAddress = true;
	}
	else if (reg == 0x03) {		// OAMADDH
		OAMAddress &= 0x00FF;
		OAMAddress |= value << 8;
		if (WrittenInternalOAMAddress) {
			if (OAMAddress & 0x100) {
				InternalOAMAddress = 0x200 | (OAMAddress & 0xFF);
			}
			else {
				InternalOAMAddress = OAMAddress & 0xFF;
			}
			WrittenInternalOAMAddress = false;
		}
	}
	else if (reg == 0x04) {		// OAMDATA
		if (InternalOAMAddress % 2 == 0) {
			OAM_Lsb = value;
		}
		else if (InternalOAMAddress % 2 == 1 && InternalOAMAddress < 0x200){
			if ((InternalOAMAddress & 0x3) == 0x1) {
				objs[InternalOAMAddress / 4].posX &= 0x100;
				objs[InternalOAMAddress / 4].posX |= OAM_Lsb;
				objs[InternalOAMAddress / 4].posY = value;
			}
			else {
				objs[InternalOAMAddress / 4].tileID = OAM_Lsb;
				objs[InternalOAMAddress / 4].flipY = value & 0x80;
				objs[InternalOAMAddress / 4].flipX = value & 0x40;
				objs[InternalOAMAddress / 4].priority = (value & 0x30) >> 4;
				objs[InternalOAMAddress / 4].palette = (value & 0x0E) >> 1;
				objs[InternalOAMAddress / 4].tileID |= (value & 0x01) << 8;
			}
		}
		if (InternalOAMAddress >= 0x200) {
			uint8_t index = InternalOAMAddress - 0x200;
			for (uint8_t i = 0; i < 4; i++) {
				uint8_t attr = (value >> (2 * i)) & 0x3;
				objs[i + (index * 4)].posX &= 0x00FF;
				objs[i + (index * 4)].posX |= (attr & 0x1) << 8;
				objs[i + (index * 4)].size = attr >> 1;
			}
		}

		InternalOAMAddress++;
		InternalOAMAddress %= 0x220;
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
		BG1HScroll = (value << 8) | (BG_Old_1 & ~0x07) | (BG_Old_2 & 0x07);
		BG1HScroll &= 0x03FF;
		BG_Old_1 = value;
		BG_Old_2 = value;
		M7HOFS = (value << 8) | prevM;
		M7HOFS &= 0x1FFF;
		if (M7HOFS & 0x1000) {
			M7HOFS |= 0xE000;
		}
		prevM = value;
	}
	else if (reg == 0x0E) {		// BG1VOFS or M7VOFS
		BG1VScroll = (value << 8) | BG_Old_1;
		BG1VScroll &= 0x03FF;
		BG_Old_1 = value;
		M7VOFS = (value << 8) | prevM;
		M7VOFS &= 0x1FFF;
		if (M7VOFS & 0x1000) {
			M7VOFS |= 0xE000;
		}
		prevM = value;
	}
	else if (reg == 0x0F) {		// BG2HOFS
		BG2HScroll = (value << 8) | (BG_Old_1 & ~0x07) | (BG_Old_2 & 0x07);
		BG2HScroll &= 0x03FF;
		BG_Old_1 = value;
		BG_Old_2 = value;
	}
	else if (reg == 0x10) {		// BG2VOFS
		BG2VScroll = (value << 8) | BG_Old_1;
		BG2VScroll &= 0x03FF;
		BG_Old_1 = value;
	}
	else if (reg == 0x11) {		// BG3HOFS
		BG3HScroll = (value << 8) | (BG_Old_1 & ~0x07) | (BG_Old_2 & 0x07);
		BG3HScroll &= 0x03FF;
		BG_Old_1 = value;
		BG_Old_2 = value;
	}
	else if (reg == 0x12) {		// BG3VOFS
		BG3VScroll = (value << 8) | BG_Old_1;
		BG3VScroll &= 0x03FF;
		BG_Old_1 = value;
	}
	else if (reg == 0x13) {		// BG4HOFS
		BG4HScroll = (value << 8) | (BG_Old_1 & ~0x07) | (BG_Old_2 & 0x07);
		BG4HScroll &= 0x03FF;
		BG_Old_1 = value;
		BG_Old_2 = value;
	}
	else if (reg == 0x14) {		// BG4VOFS
		BG4VScroll = (value << 8) | BG_Old_1;
		BG4VScroll &= 0x03FF;
		BG_Old_1 = value;
	}
	else if (reg == 0x15) {		// VMAIN
		VMain = value;
	}
	else if (reg == 0x16) {		// VMAADDL
		VRAMAddress &= 0xFF00;
		VRAMAddress |= value;
		VRAMAddressReset = true;
	}
	else if (reg == 0x17) {		// VMADDH
		VRAMAddress &= 0x00FF;
		VRAMAddress |= value << 8;
		VRAMAddressReset = true;
	}
	else if (reg == 0x18) {		// VMDATAL
		uint16_t remapAddress = VRAMAddress;
		if ((VMain & 0xC) != 0) {
			if ((VMain & 0xC) == 0x4) {
				uint8_t lower = VRAMAddress & 0xFF;
				remapAddress &= 0xFF00;
				remapAddress |= (lower >> 5) | (lower << 3 & 0x00F8);
			}
			else if ((VMain & 0xC) == 0x8) {
				uint16_t lower = VRAMAddress & 0x1FF;
				remapAddress &= 0xFE00;
				remapAddress |= (lower >> 6) | (lower << 3 & 0x01F8);
			}
			else if ((VMain & 0xC) == 0xC) {
				uint16_t lower = VRAMAddress & 0x3FF;
				remapAddress &= 0xFC00;
				remapAddress |= (lower >> 7) | (lower << 3 & 0x03F8);
			}
		}
		remapAddress %= 0x8000;
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
		}
	}
	else if (reg == 0x19) {		// VMDATAH
		uint16_t remapAddress = VRAMAddress;
		if ((VMain & 0xC) != 0) {
			if ((VMain & 0xC) == 0x4) {
				uint8_t lower = VRAMAddress & 0xFF;
				remapAddress &= 0xFF00;
				remapAddress |= (lower >> 5) | (lower << 3 & 0x00F8);
			}
			else if ((VMain & 0xC) == 0x8) {
				uint16_t lower = VRAMAddress & 0x1FF;
				remapAddress &= 0xFE00;
				remapAddress |= (lower >> 6) | (lower << 3 & 0x01F8);
			}
			else if ((VMain & 0xC) == 0xC) {
				uint16_t lower = VRAMAddress & 0x3FF;
				remapAddress &= 0xFC00;
				remapAddress |= (lower >> 7) | (lower << 3 & 0x03F8);
			}
		}
		remapAddress %= 0x8000;
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
		}
	}
	else if (reg == 0x1A) {		// M7SEL
		M7SEL = value;
		assert((value & 0x03) == 0);
		assert((value & 0x40) == 0);
	}
	else if (reg == 0x1B) {		// M7A
		M7A = (value << 8) | prevM;
		prevM = value;
		multResult = M7A * (M7B >> 8);
	}
	else if (reg == 0x1C) {		// M7B
		M7B = (value << 8) | prevM;
		prevM = value;
		multResult = M7A * (M7B >> 8);
	}
	else if (reg == 0x1D) {		// M7C
		M7C = (value << 8) | prevM;
		prevM = value;
	}
	else if (reg == 0x1E) {		// M7D
		M7D = (value << 8) | prevM;
		prevM = value;
	}
	else if (reg == 0x1F) {		// M7X
		M7X = (value << 8) | prevM;
		M7X &= 0x1FFF;
		if (M7X & 0x1000) {
			M7X |= 0xE000;
		}
		prevM = value;
	}
	else if (reg == 0x20) {		// M7Y
		M7Y = (value << 8) | prevM;
		M7Y &= 0x1FFF;
		if (M7Y & 0x1000) {
			M7Y |= 0xE000;
		}
		prevM = value;
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
	else if (reg == 0x23) {		// W12SEL
		W12SEL = value;
	}
	else if (reg == 0x24) {		// W34SEL
		W34SEL = value;
	}
	else if (reg == 0x25) {		// WOBJSEL
		WOBJSEL = value;
	}
	else if (reg == 0x26) {		// WH0
		window1Left = value;
	}
	else if (reg == 0x27) {		// WH1
		window1Right = value;
	}
	else if (reg == 0x28) {		// WH2
		window2Left = value;
	}
	else if (reg == 0x29) {		// WH3
		window2Right = value;
	}
	else if (reg == 0x2A) {		// WBGLOG
		WBGLOG = value;
	}
	else if (reg == 0x2B) {		// WOBJLOG
		WOBJLOG = value;
	}
	else if (reg == 0x2C) {		// TM
		TM = value;
	}
	else if (reg == 0x2D) {		// TS
		TS = value;
	}
	else if (reg == 0x2E) {		// TMW
		TMW = value;
	}
	else if (reg == 0x2F) {		// TSW
		TSW = value;
	}
	else if (reg == 0x30) {		// CGWSEL
		CGWSEL = value;
		assert((CGWSEL & 0x01) == 0 && "CGWSEL Direct Color");
	}
	else if (reg == 0x31) {		// CGADSUB
		CGADSUB = value;
	}
	else if (reg == 0x32) {		// COLDATA
		if (value & 0x20) {
			fixedColor &= ~0x1F;
			fixedColor |= value & 0x1F;
		}
		if (value & 0x40) {
			fixedColor &= ~(0x1F << 5);
			fixedColor |= (value & 0x1F) << 5;
		}
		if (value & 0x80) {
			fixedColor &= ~(0x1F << 10);
			fixedColor |= (value & 0x1F) << 10;
		}
	}
	else if (reg == 0x33) {
		printf("\033[0;32mWritting SETINI: %02x\033[0m\n", value);
		overscan = value & 0x04;
		assert((value & ~0x04) == 0);
	}
	else if (reg == 0x34) {
		printf("\033[0;32mWritting MPYL (read only): %02x\033[0m\n", value);
	}
	else if (reg >= 0x35 && reg <= 0x3F) {
		// Read only
	}
	else {
		printf("%06x: %02x\n", address, value);
		assert(false && "Writting PPU");
	}
}

void PPU::SetPixel(int X, int Y) {
	if (inidisp & 0x80) {
		pixels[(Y * width) + X] = 0x0000;
		return;
	}
	// Backdrop
	mainPixelPipeline[5].color = CGRAM[0];
	mainPixelPipeline[5].priority = 0;
	mainPixelPipeline[5].id = 5;
	subPixelPipeline[5].color = fixedColor;
	subPixelPipeline[5].priority = 0;
	subPixelPipeline[5].id = 5;

	// BG4
	mainPixelPipeline[3].color = 0;
	mainPixelPipeline[3].priority = 0;
	mainPixelPipeline[3].id = 3;
	subPixelPipeline[3].color = 0;
	subPixelPipeline[3].priority = 0;
	subPixelPipeline[3].id = 3;

	if (((TM & 0x08) || (TS & 0x08)) && BG4Display) {
		SetBGPixel(X, Y, BG4TileAddress, 3, GetColorBPPForBG(3),
				(BG4TileAddress & 0x1) ? 64 : 32, (BG4TileAddress & 0x2) ? 64 : 32,
				BG4HScroll, BG4VScroll + 1);
	}
	// Window BG4
	if (IsInsideWindow(X, 3, TMW)) {
		mainPixelPipeline[3].color = 0;
	}
	if (IsInsideWindow(X, 3, TSW)) {
		subPixelPipeline[3].color = 0;
	}

	// BG3
	mainPixelPipeline[2].color = 0;
	mainPixelPipeline[2].priority = 0;
	mainPixelPipeline[2].id = 2;
	subPixelPipeline[2].color = 0;
	subPixelPipeline[2].priority = 0;
	subPixelPipeline[2].id = 2;

	if (((TM & 0x04) || (TS & 0x04)) && BG3Display) {
		SetBGPixel(X, Y, BG3TileAddress, 2, GetColorBPPForBG(2),
				(BG3TileAddress & 0x1) ? 64 : 32, (BG3TileAddress & 0x2) ? 64 : 32,
				BG3HScroll, BG3VScroll + 1);
	}
	// Window BG3
	if (IsInsideWindow(X, 2, TMW)) {
		mainPixelPipeline[2].color = 0;
	}
	if (IsInsideWindow(X, 2, TSW)) {
		subPixelPipeline[2].color = 0;
	}

	// BG2
	mainPixelPipeline[1].color = 0;
	mainPixelPipeline[1].priority = 0;
	mainPixelPipeline[1].id = 1;
	subPixelPipeline[1].color = 0;
	subPixelPipeline[1].priority = 0;
	subPixelPipeline[1].id = 1;

	if (((TM & 0x02) || (TS & 0x02)) && BG2Display) {
		SetBGPixel(X, Y, BG2TileAddress, 1, GetColorBPPForBG(1),
			(BG2TileAddress & 0x1) ? 64 : 32, (BG2TileAddress & 0x2) ? 64 : 32,
			BG2HScroll, BG2VScroll + 1);
	}
	// Window BG2
	if (IsInsideWindow(X, 1, TMW)) {
		mainPixelPipeline[1].color = 0;
	}
	if (IsInsideWindow(X, 1, TSW)) {
		subPixelPipeline[1].color = 0;
	}

	// BG1
	mainPixelPipeline[0].color = 0;
	mainPixelPipeline[0].priority = 0;
	mainPixelPipeline[0].id = 0;
	subPixelPipeline[0].color = 0;
	subPixelPipeline[0].priority = 0;
	subPixelPipeline[0].id = 0;
	
	if (((TM & 0x01) || (TS & 0x01)) && BG1Display) {
		if ((BGMode & 0x7) != 7) {
			SetBGPixel(X, Y, BG1TileAddress, 0, GetColorBPPForBG(0),
					(BG1TileAddress & 0x1) ? 64 : 32, (BG1TileAddress & 0x2) ? 64 : 32,
					BG1HScroll, BG1VScroll + 1);
		}
		else {
			SetBG1PixelMode7(X, Y);
		}
	}
	// Window BG1
	if (IsInsideWindow(X, 0, TMW)) {
		mainPixelPipeline[0].color = 0;
	}
	if (IsInsideWindow(X, 0, TSW)) {
		subPixelPipeline[0].color = 0;
	}

	// OBJs
	mainPixelPipeline[4].color = 0;
	mainPixelPipeline[4].priority = 0;
	mainPixelPipeline[4].id = 4;
	mainPixelPipeline[4].palette = 0;
	subPixelPipeline[4].color = 0;
	subPixelPipeline[4].priority = 0;
	subPixelPipeline[4].id = 4;
	subPixelPipeline[4].palette = 0;
	if (((TM & 0x10) || (TS & 0x10)) && ObjDisplay) {
		SetOBJPixel(X, Y);
	}
	// Window OBJ
	if (IsInsideWindow(X, 4, TMW)) {
		mainPixelPipeline[4].color = 0;
	}
	if (IsInsideWindow(X, 4, TSW)) {
		subPixelPipeline[4].color = 0;
	}

	// Get Priority Pixel
	PPU::Pixel mainPixel = GetPixelByPriority(mainPixelPipeline);
	PPU::Pixel subPixel = (CGWSEL & 0x2) == 0 ? subPixelPipeline[5] : GetPixelByPriority(subPixelPipeline);

	// MainSW
	if ((CGWSEL & 0xC0) == 0xC0) {
		// Set color to black always
		mainPixel.color = 0;
	}
	else if (CGWSEL & 0x80) {
		// Set color to black if inside window
		if (IsInsideWindow(X, 5, 0xFF)) {
			mainPixel.color = 0;
		}
	}
	else if (CGWSEL & 0x40) {
		// Set color to black if outside window
		if (!IsInsideWindow(X, 5, 0xFF)) {
			mainPixel.color = 0;
		}
	}

	// SubSW
	if (!(mainPixel.id == 4 && mainPixel.palette < 4)) {		// OBJS with palettes 0-3 do not Math Color
		if ((CGWSEL & 0x30) == 0) {
			// Never prevent color math
			// Color Math
			if (CGADSUB & (1 << mainPixel.id)) {
				mainPixel.color = ColorMathPixel(mainPixel.color, subPixel);
			}
		}
		else if (CGWSEL & 0x20) {
			// Prevent color math inside window
			if (!IsInsideWindow(X, 5, 0xFF)) {
				// Color Math
				if (CGADSUB & (1 << mainPixel.id)) {
					mainPixel.color = ColorMathPixel(mainPixel.color, subPixel);
				}
			}
		}
		else if (CGWSEL & 0x10) {
			// Prevent color math outside window
			if (IsInsideWindow(X, 5, 0xFF)) {
				// Color Math
				if (CGADSUB & (1 << mainPixel.id)) {
					mainPixel.color = ColorMathPixel(mainPixel.color, subPixel);
				}
			}
		}
	}

	// Apply brightness
	if ((inidisp & 0xF) != 0xF) {
		double brightness = (inidisp & 0xF) / (double)0xF;
		uint8_t red = mainPixel.color & 0x1F;
		uint8_t green = (mainPixel.color >> 5) & 0x1F;
		uint8_t blue = (mainPixel.color >> 10) & 0x1F;
		mainPixel.color &= ~0x1F;
		mainPixel.color |= (uint8_t)(red * brightness) & 0x1F;
		mainPixel.color &= ~(0x1F << 5);
		mainPixel.color |= ((uint8_t)(green * brightness) & 0x1F) << 5;
		mainPixel.color &= ~(0x1F << 10);
		mainPixel.color |= ((uint8_t)(blue * brightness) & 0x1F) << 10;
	}

	pixels[(Y * width) + X] = mainPixel.color;
}

void PPU::SetBGPixel(int X, int Y, uint8_t BGAddress, uint8_t BGIndex, uint8_t bpp,
						uint8_t XSize, uint8_t YSize,
						uint16_t HScroll, uint16_t VScroll) {
	uint8_t spriteSize = 8;
	if (BGMode & (0x10 << BGIndex)) {
		spriteSize = 16;
	}

	int XScrolled = (X + HScroll) % (spriteSize * XSize);
	int YScrolled = (Y + VScroll + (overscan ? 8 : 0)) % (spriteSize * YSize);

	// Mod 0x8000 because we use 16bits values instead of 8bits
	uint16_t baseAddress = ((BGAddress >> 2) << 10) % 0x8000;

	uint8_t TileX = XScrolled / spriteSize;
	uint8_t TileY = YScrolled / spriteSize;

	uint16_t rowSize = 0xFF + 0x100 * (spriteSize / 16);
	uint16_t offset = (((YScrolled & rowSize) / spriteSize) << 5) + ((XScrolled & rowSize) / spriteSize);
	// Right map
	if (TileX >= 32) {
		offset += (XSize == 64) ? 0x400 : 0;
	}
	// Down map
	if (TileY >= 32) {
		if (YSize == 64) {
			if (XSize == 64) {
				offset += 0x800;
			}
			else {
				offset += 0x400;
			}
		}
	}

	uint16_t tileWord = VRAM[baseAddress + offset];

	uint16_t tileID = tileWord & 0x3FF;
	uint8_t BGPaletteNumber = (tileWord >> 10) & 0x7;
	bool BGPriority = tileWord & 0x2000;
	bool BGFlipX = tileWord & 0x4000;
	bool BGFlipY = tileWord & 0x8000;

	if (spriteSize == 16) {
		if (XScrolled % 16 >= 8) {
			tileID += 1;
		}
		if (YScrolled % 16 >= 8) {
			tileID += 16;
		}
	}

	uint8_t i = YScrolled % 8;
	uint8_t j = XScrolled % 8;
	uint8_t VShift = i + (-2 * i + 7) * BGFlipY;
	uint8_t HShift = (7 - j) + (2 * j - 7) * BGFlipX;

	uint16_t tileOffset = 0;
	if (BGIndex == 0) {
		tileOffset = (BGAddress1_2 & 0x0F) << 12;
	}
	else if (BGIndex == 1) {
		tileOffset = (BGAddress1_2 & 0xF0) << 8;
	}
	else if (BGIndex == 2) {
		tileOffset = (BGAddress3_4 & 0x0F) << 12;
	}
	else if (BGIndex == 3) {
		tileOffset = (BGAddress3_4 & 0xF0) << 8;
	}
	tileOffset %= 0x8000;

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

	uint8_t mosaicSize = 0;
	if (mosaic & (1 << BGIndex) && (mosaic & 0xF0) >= 0x10) {
		mosaicSize = mosaic >> 4;
	}
	if (mosaicSize >= 1) {
		mosaicSize++;
		if (Y % mosaicSize != 0 || X % mosaicSize != 0) {
			palette = pixels[((Y - (Y % mosaicSize)) * width) + (X - (X % mosaicSize))];
		}
	}

	// Main Screen
	if (TM & (1 << BGIndex)) {
		mainPixelPipeline[BGIndex].color = palette;
		mainPixelPipeline[BGIndex].priority = BGPriority ? 1 : 0;
		mainPixelPipeline[BGIndex].id = BGIndex;
	}
	// Subscreen
	if (TS & (1 << BGIndex)) {
		subPixelPipeline[BGIndex].color = palette;
		subPixelPipeline[BGIndex].priority = BGPriority ? 1 : 0;
		subPixelPipeline[BGIndex].id = BGIndex;
	}
}

void PPU::SetOBJPixel(int X, int Y) {
	PPU::Pixel objPixel;
	objPixel.color = 0;
	objPixel.priority = 0;
	objPixel.palette = 0;

	int index = 0;
	PPU::OBJECT obj;
	for (int i = 127; i >= 0; i--) {
		index = objsToDisplay[i];
		if (index == -1) {
			break;
		}
		obj = objs[index];
		if (IsOBJOnScreen(X, Y, obj, objPixel.priority)) {
			uint8_t objX = X - obj.posX;
			uint8_t objY = Y - obj.posY;

			uint8_t objWidth = ObjSizes[obj.size ? 1 : 0][OBSEL >> 5];

			if (obj.flipX) {
				objX = objWidth - objX - 1;
			}
			if (obj.flipY) {
				objY = objWidth - objY - 1;
			}

			uint8_t offsetX = 7 - (objX % 8);
			uint16_t baseAddress = (OBSEL & 0x7) << 13;
			uint16_t tileAddress = baseAddress + (obj.tileID + objX / 8) * 0x10 + (objY % 8) + (objY / 8 * 0x100);
			uint16_t color = GetColor4BPP(tileAddress, obj.palette + 8, offsetX);

			if (color & 0x8000) {
				objPixel.color = color;
				objPixel.priority = obj.priority;
				objPixel.palette = obj.palette;
			}
		}
	}
	
	if (TM & 0x10) {
		mainPixelPipeline[4].color = objPixel.color;
		mainPixelPipeline[4].priority = objPixel.priority;
		mainPixelPipeline[4].palette = objPixel.palette;
	}
	if (TS & 0x10) {
		subPixelPipeline[4].color = objPixel.color;
		subPixelPipeline[4].priority = objPixel.priority;
		subPixelPipeline[4].palette = objPixel.palette;
	}
}

bool PPU::IsOBJOnScreen(int X, int Y, PPU::OBJECT& obj, uint8_t currentPriority) {
	if (X >= obj.maxX) {
		return false;
	}
	if (Y >= obj.maxY) {
		return false;
	}
	if (obj.clipping == 3) {
		return obj.priority >= currentPriority;
	}
	if (X < obj.posX && obj.clipping != 2) {
		return false;
	}
	if (Y < obj.posY && obj.clipping != 1) {
		return false;
	}
	return obj.priority >= currentPriority;
}

void PPU::OBJReloadSize() {
	uint8_t size = 0;
	for (uint8_t i = 0; i < 128; i++) {
		size = ObjSizes[objs[i].size ? 1 : 0][OBSEL >> 5];
		if (overscan) {
			objs[i].posY -= 8;
		}
		objs[i].maxX = (objs[i].posX + size) & 0x1FF;
		objs[i].maxY = objs[i].posY + size;
		objs[i].clipping = 0;
	}
}

void PPU::GetOBJsToDisplay() {
	int pointer = 127;
	for (int i = 127; i >= 0; i--) {
		if (objs[i].posY < height && objs[i].posX < width) {
			objsToDisplay[pointer--] = i;
			continue;
		}
		if (objs[i].maxY != 0 && objs[i].maxY < objs[i].posY && objs[i].posX < width) {
			objs[i].clipping = 1;
			objsToDisplay[pointer--] = i;
			continue;
		}
		if (objs[i].maxX != 0 && objs[i].maxX < objs[i].posX && objs[i].posY < height) {
			objs[i].clipping = 2;
			objsToDisplay[pointer--] = i;
			continue;
		}
		if (objs[i].maxX != 0 && objs[i].maxY != 0 && objs[i].maxX < objs[i].posX && objs[i].maxY < objs[i].posY) {
			objs[i].clipping = 3;
			objsToDisplay[pointer--] = i;
			continue;
		}
	}
	objsToDisplay[pointer] = -1;
}

void PPU::SetBG1PixelMode7(int X, int Y) {
	CalculateMode7Coor(X, Y);

	bool outside = false;
	if ((M7SEL & 0x80) == 0) {
		X &= 0x3FF;
		Y &= 0x3FF;
	} else {
		if (M7SEL & 0x40) {
			// Fill with character 0
		} else {
			if (X < 0 || X > 0x3FF) {
				outside = true;
			}
			if (Y < 0 || Y > 0x3FF) {
				outside = true;
			}
		}
	}

	uint16_t tileID = ((Y >> 3) << 7) | (X >> 3);
	tileID = VRAM[tileID] & 0xFF;

	uint16_t palette = (tileID << 6) | ((Y & 0x7) << 3) | (X & 0x7);
	palette = VRAM[palette] >> 8;

	if (palette == 0 || outside) {
		palette = CGRAM[0];
	}
	else {
		palette = CGRAM[palette] | 0x8000;
	}

	uint8_t mosaicSize = 0;
	if (mosaic & (1 << 0) && (mosaic & 0xF0) >= 0x10) {
		mosaicSize = mosaic >> 4;
	}
	if (mosaicSize >= 1) {
		mosaicSize++;
		if (Y % mosaicSize != 0 || X % mosaicSize != 0) {
			palette = pixels[((Y - (Y % mosaicSize)) * width) + (X - (X % mosaicSize))];
		}
	}

	// Main Screen
	if (TM & (1 << 0)) {
		mainPixelPipeline[0].color = palette;
		mainPixelPipeline[0].priority = 0;
		mainPixelPipeline[0].id = 0;
	}
	else {
		mainPixelPipeline[0].color = 0;
		mainPixelPipeline[0].priority = 0;
		mainPixelPipeline[0].id = 0;
	}
	// Subscreen
	if (TS & (1 << 0)) {
		subPixelPipeline[0].color = palette;
		subPixelPipeline[0].priority = 0;
		subPixelPipeline[0].id = 0;
	}
	else {
		subPixelPipeline[0].color = 0;
		subPixelPipeline[0].priority = 0;
		subPixelPipeline[0].id = 0;
	}
}

void PPU::CalculateMode7Coor(int& X, int& Y) const {
	// TODO: Check if flip

	#define CLIP(a) (((a) & 0x2000) ? ((a) | ~0x3FF) : ((a) & 0x3FF))

	int originX = (M7A * CLIP(M7HOFS - M7X) & ~0x3F) + (M7B * CLIP(M7VOFS - M7Y) & ~0x3F) + (M7B * Y & ~0x3F) + (M7X << 8);
	int originY = (M7C * CLIP(M7HOFS - M7X) & ~0x3F) + (M7D * CLIP(M7VOFS - M7Y) & ~0x3F) + (M7D * Y & ~0x3F) + (M7Y << 8);

	Y = originY + M7C * X >> 8;
	X = originX + M7A * X >> 8;
}

uint8_t PPU::GetColorBPPForBG(uint8_t BGIndex) const {
	uint8_t mode = BGMode & 0x07;

	if (mode == 0) {
		return 2;
	}
	if (mode == 1) {
		if (BGIndex == 2) {
			return 2;
		}
		return 4;
	}
	if (mode == 2) {
		return 4;
	}
	if (mode == 3) {
		if (BGIndex == 0) {
			return 8;
		}
		return 4;
	}
	if (mode == 7) {
		if (BGIndex == 1) {
			return 8;
			assert(false);
		}
		return 8;
	}
	assert(false && "BG Mode not found.");
	return 0;
}

uint16_t PPU::GetColor2BPP(uint16_t charAddress, uint8_t palette, uint8_t HShift, uint8_t BGIndex) const {
	uint8_t BGLo = VRAM[charAddress] & 0xFF;
	uint8_t BGHi = VRAM[charAddress] >> 8;
	uint8_t paletteIndex = ((BGLo >> HShift) & 1) + (2 * ((BGHi >> HShift) & 1));

	if (paletteIndex == 0) {
		return CGRAM[0];
	}
	uint8_t mode = BGMode & 0x07;
	if (mode == 0) {
		return CGRAM[paletteIndex + palette * 4 + BGIndex * 32] | 0x8000;
	}
	return CGRAM[paletteIndex + palette * 4] | 0x8000;
}

uint16_t PPU::GetColor4BPP(uint16_t charAddress, uint8_t palette, uint8_t HShift) const {
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

uint16_t PPU::GetColor8BPP(uint16_t charAddress, uint8_t palette, uint8_t HShift) const {
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

uint16_t PPU::ColorMathPixel(uint16_t palette, PPU::Pixel subpixel) const {
	uint8_t red = palette & 0x1F;
	uint8_t green = (palette >> 5) & 0x1F;
	uint8_t blue = (palette >> 10) & 0x1F;
	uint8_t redSub = subpixel.color & 0x1F;
	uint8_t greenSub = (subpixel.color >> 5) & 0x1F;
	uint8_t blueSub = (subpixel.color >> 10) & 0x1F;
	bool half = (CGADSUB & 0x40);
	if ((TS & 0x1F) == 0 && (CGWSEL & 0x2)) {
		half = false;
	}
	if (CGADSUB & 0x80) {
		palette &= ~0x1F;
		if (red - redSub > 0) {
			palette |= ((red - redSub) & 0x1F) / (half ? 2 : 1);
		}
		palette &= ~(0x1F << 5);
		if (green - greenSub > 0) {
			palette |= (((green - greenSub) & 0x1F) / (half ? 2 : 1)) << 5;
		}
		palette &= ~(0x1F << 10);
		if (blue - blueSub > 0) {
			palette |= (((blue - blueSub) & 0x1F) / (half ? 2 : 1)) << 10;
		}
	}
	else {
		palette &= ~0x1F;
		uint8_t temp = (red + redSub) / (half ? 2 : 1);
		if (temp > 0x1F) {
			palette |= 0x1F;
		}
		else {
			palette |= temp;
		}
		palette &= ~(0x1F << 5);
		temp = (green + greenSub) / (half ? 2 : 1);
		if (temp > 0x1F) {
			palette |= 0x1F << 5;
		}
		else {
			palette |= temp << 5;
		}
		palette &= ~(0x1F << 10);
		temp = (blue + blueSub) / (half ? 2 : 1);
		if (temp > 0x1F) {
			palette |= 0x1F << 10;
		}
		else {
			palette |= temp << 10;
		}
	}
	return palette;
}

bool PPU::IsInsideWindow(int X, uint8_t BGIndex, uint8_t enableReg) const {
	bool window1Enable = false;
	bool isInsideWindow1 = false;
	bool window2Enable = false;
	bool isInsideWindow2 = false;
	if (BGIndex == 0 || BGIndex == 1) {
		uint8_t offset = (BGIndex == 0) ? 0 : 4;
		if (W12SEL & (0x2 << offset)) {
			window1Enable = true;
			if (window1Left > window1Right) {
				isInsideWindow1 = false;
			}
			else if (X >= window1Left && X <= window1Right) {
				isInsideWindow1 = true;
			}
			if (W12SEL & (0x1 << offset)) {
				isInsideWindow1 = !isInsideWindow1;
			}
		}
		if (W12SEL & (0x8 << offset)) {
			window2Enable = true;
			if (window2Left > window2Right) {
				isInsideWindow2 = false;
			}
			else if (X >= window2Left && X <= window2Right) {
				isInsideWindow2 = true;
			}
			if (W12SEL & (0x4 << offset)) {
				isInsideWindow2 = !isInsideWindow2;
			}
		}
	}
	else if (BGIndex == 2 || BGIndex == 3) {
		uint8_t offset = (BGIndex == 2) ? 0 : 4;
		if (W34SEL & (0x2 << offset)) {
			window1Enable = true;
			if (window1Left > window1Right) {
				isInsideWindow1 = false;
			}
			else if (X >= window1Left && X <= window1Right) {
				isInsideWindow1 = true;
			}
			if (W34SEL & (0x1 << offset)) {
				isInsideWindow1 = !isInsideWindow1;
			}
		}
		if (W34SEL & (0x8 << offset)) {
			window2Enable = true;
			if (window2Left > window2Right) {
				isInsideWindow2 = false;
			}
			else if (X >= window2Left && X <= window2Right) {
				isInsideWindow2 = true;
			}
			if (W34SEL & (0x4 << offset)) {
				isInsideWindow2 = !isInsideWindow2;
			}
		}
	}
	else if (BGIndex == 4 || BGIndex == 5) {
		uint8_t offset = (BGIndex == 4) ? 0 : 4;
		if (WOBJSEL & (0x2 << offset)) {
			window1Enable = true;
			if (window1Left > window1Right) {
				isInsideWindow1 = false;
			}
			else if (X >= window1Left && X <= window1Right) {
				isInsideWindow1 = true;
			}
			if (WOBJSEL & (0x1 << offset)) {
				isInsideWindow1 = !isInsideWindow1;
			}
		}
		if (WOBJSEL & (0x8 << offset)) {
			window2Enable = true;
			if (window2Left > window2Right) {
				isInsideWindow2 = false;
			}
			else if (X >= window2Left && X <= window2Right) {
				isInsideWindow2 = true;
			}
			if (WOBJSEL & (0x4 << offset)) {
				isInsideWindow2 = !isInsideWindow2;
			}
		}
	}
	if (!window1Enable && !window2Enable) {
		return false;
	}

	if ((enableReg & (1 << BGIndex)) == 0) {
		return false;
	}

	if (!window2Enable) {
		return isInsideWindow1;
	}
	if (!window1Enable) {
		return isInsideWindow2;
	}
	uint8_t operation = 0;
	if (BGIndex < 4) {
		operation = (WBGLOG & (0x3 << (2 * BGIndex))) >> (2 * BGIndex);
	} else {
		operation = (WOBJLOG & (0x3 << (2 * (BGIndex - 4)))) >> (2 * (BGIndex - 4));
	}
	if (operation == 0x00) {
		return isInsideWindow1 || isInsideWindow2;
	}
	else if (operation == 0x01) {
		return isInsideWindow1 && isInsideWindow2;
	}
	else if (operation == 0x02) {
		return isInsideWindow1 ^ isInsideWindow2;
	}
	else if (operation == 0x03) {
		return !(isInsideWindow1 ^ isInsideWindow2);
	}
	return false;
}

PPU::Pixel PPU::GetPixelByPriority(PPU::Pixel* pixels) const {
	uint8_t mode = BGMode & 0x07;
	switch (mode) {
		// Check pixels from front to back
	case 0:
	{
		// Sprites with priority 3
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 3)
			return pixels[4];
		// BG1 tiles with priority 1
		if ((pixels[0].color & 0x8000) && pixels[0].priority == 1)
			return pixels[0];
		// BG2 tiles with priority 1
		if ((pixels[1].color & 0x8000) && pixels[1].priority == 1)
			return pixels[1];
		// Sprites with priority 2
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 2)
			return pixels[4];
		// BG1 tiles with priority 0
		if ((pixels[0].color & 0x8000) && pixels[0].priority == 0)
			return pixels[0];
		// BG2 tiles with priority 0
		if ((pixels[1].color & 0x8000) && pixels[1].priority == 0)
			return pixels[1];
		// Sprites with priority 1
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 1)
			return pixels[4];
		// BG3 tiles with priority 1
		if ((pixels[2].color & 0x8000) && pixels[2].priority == 1)
			return pixels[2];
		// BG4 tiles with priority 1
		if ((pixels[3].color & 0x8000) && pixels[3].priority == 1)
			return pixels[3];
		// Sprites with priority 0
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 0)
			return pixels[4];
		// BG3 tiles with priority 0
		if ((pixels[2].color & 0x8000) && pixels[2].priority == 0)
			return pixels[2];
		// BG4 tiles with priority 0
		if ((pixels[3].color & 0x8000) && pixels[3].priority == 0)
			return pixels[3];

		break;
	}

	case 1:
	{
		bool BG3Priority = BGMode & 0x08;
		// BG3 tiles with priority 1 if bit 3 of $2105 is set
		if ((pixels[2].color & 0x8000) && pixels[2].priority == 1 && BG3Priority)
			return pixels[2];
		// Sprites with priority 3
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 3)
			return pixels[4];
		// BG1 tiles with priority 1
		if ((pixels[0].color & 0x8000) && pixels[0].priority == 1)
			return pixels[0];
		// BG2 tiles with priority 1
		if ((pixels[1].color & 0x8000) && pixels[1].priority == 1)
			return pixels[1];
		// Sprites with priority 2
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 2)
			return pixels[4];
		// BG1 tiles with priority 0
		if ((pixels[0].color & 0x8000) && pixels[0].priority == 0)
			return pixels[0];
		// BG2 tiles with priority 0
		if ((pixels[1].color & 0x8000) && pixels[1].priority == 0)
			return pixels[1];
		// Sprites with priority 1
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 1)
			return pixels[4];
		// BG3 tiles with priority 1 if bit 3 of $2105 is clear
		if ((pixels[2].color & 0x8000) && pixels[2].priority == 1 && !BG3Priority)
			return pixels[2];
		// Sprites with priority 0
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 0)
			return pixels[4];
		// BG3 tiles with priority 0
		if ((pixels[2].color & 0x8000) && pixels[2].priority == 0)
			return pixels[2];

		break;
	}

	case 2:
	{
		// Sprites with priority 3
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 3)
			return pixels[4];
		// BG1 tiles with priority 1
		if ((pixels[0].color & 0x8000) && pixels[0].priority == 1)
			return pixels[0];
		// Sprites with priority 2
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 2)
			return pixels[4];
		// BG2 tiles with priority 1
		if ((pixels[1].color & 0x8000) && pixels[1].priority == 1)
			return pixels[1];
		// Sprites with priority 1
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 1)
			return pixels[4];
		// BG1 tiles with priority 0
		if ((pixels[0].color & 0x8000) && pixels[0].priority == 0)
			return pixels[0];
		// Sprites with priority 0
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 0)
			return pixels[4];
		// BG2 tiles with priority 0
		if ((pixels[1].color & 0x8000) && pixels[1].priority == 0)
			return pixels[1];

		break;
	}

	case 3:
	{
		// Sprites with priority 3
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 3)
			return pixels[4];
		// BG1 tiles with priority 1
		if ((pixels[0].color & 0x8000) && pixels[0].priority == 1)
			return pixels[0];
		// Sprites with priority 2
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 2)
			return pixels[4];
		// BG2 tiles with priority 1
		if ((pixels[1].color & 0x8000) && pixels[1].priority == 1)
			return pixels[1];
		// Sprites with priority 1
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 1)
			return pixels[4];
		// BG1 tiles with priority 0
		if ((pixels[0].color & 0x8000) && pixels[0].priority == 0)
			return pixels[0];
		// Sprites with priority 0
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 0)
			return pixels[4];
		// BG2 tiles with priority 0
		if ((pixels[1].color & 0x8000) && pixels[1].priority == 0)
			return pixels[1];

		break;
	}

	case 7:
	{
		// Sprites with priority 3
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 3)
			return pixels[4];
		// Sprites with priority 2
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 2)
			return pixels[4];
		// Sprites with priority 1
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 1)
			return pixels[4];
		// BG1 tiles
		if (pixels[0].color & 0x8000)
			return pixels[0];
		// Sprites with priority 0
		if ((pixels[4].color & 0x8000) && pixels[4].priority == 0)
			return pixels[4];

		break;
	}

	default:
		assert(false);
	}

	return pixels[5];
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

void PPU::GetMode7BG(void* texturePixels, int pitch) {
	uint16_t* outPixels = (uint16_t*)texturePixels;
	for (int y = 0; y < 1024; y++) {
		for (int x = 0; x < 1024; x++) {
			uint16_t tileID = ((y >> 3) << 7) | (x >> 3);
			tileID = VRAM[tileID] & 0xFF;

			uint16_t palette = (tileID << 6) | ((y & 0x7) << 3) | (x & 0x7);
			palette = VRAM[palette] >> 8;

			if (palette == 0) {
				palette = CGRAM[0] | 0x8000;
			}
			else {
				palette = CGRAM[palette] | 0x8000;
			}
			outPixels[y * (pitch / 2) + x] = palette;
		}
	}
}
