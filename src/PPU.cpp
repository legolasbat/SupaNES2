#include "PPU.h"

#include "SupaNES.h"

#include <cassert>
#include <stdio.h>

PPU::PPU(SupaNES* supaNes) {
	memory = supaNes;
}

void PPU::Clock() {

}

void PPU::WritePPU(uint32_t address, uint8_t value) {
	uint8_t reg = address & 0xFF;

	if (reg == 0x00) {
		printf("\033[0;32mWritting INIDISP: %02x\033[0m\n", value);
	}
	else if (reg == 0x01) {
		printf("\033[0;32mWritting OBSEL: %02x\033[0m\n", value);
	}
	else if (reg == 0x02) {
		printf("\033[0;32mWritting OAMADDL: %02x\033[0m\n", value);
	}
	else if (reg == 0x03) {
		printf("\033[0;32mWritting OAMADDH: %02x\033[0m\n", value);
	}
	else if (reg == 0x04) {
		printf("\033[0;32mWritting OAMDATA: %02x\033[0m\n", value);
	}
	else if (reg == 0x05) {
		printf("\033[0;32mWritting BGMODE: %02x\033[0m\n", value);
	}
	else if (reg == 0x06) {
		printf("\033[0;32mWritting MOSAIC: %02x\033[0m\n", value);
	}
	else if (reg == 0x07) {
		printf("\033[0;32mWritting BG1SC: %02x\033[0m\n", value);
	}
	else if (reg == 0x08) {
		printf("\033[0;32mWritting BG2SC: %02x\033[0m\n", value);
	}
	else if (reg == 0x09) {
		printf("\033[0;32mWritting BG3SC: %02x\033[0m\n", value);
	}
	else if (reg == 0x0A) {
		printf("\033[0;32mWritting BG4SC: %02x\033[0m\n", value);
	}
	else if (reg == 0x0B) {
		printf("\033[0;32mWritting BG12NBA: %02x\033[0m\n", value);
	}
	else if (reg == 0x0C) {
		printf("\033[0;32mWritting BG34NBA: %02x\033[0m\n", value);
	}
	else if (reg == 0x0D) {
		printf("\033[0;32mWritting BG1HOFS: %02x\033[0m\n", value);
	}
	else if (reg == 0x0E) {
		printf("\033[0;32mWritting BG1VOFS: %02x\033[0m\n", value);
	}
	else if (reg == 0x0F) {
		printf("\033[0;32mWritting BG2HOFS: %02x\033[0m\n", value);
	}
	else if (reg == 0x10) {
		printf("\033[0;32mWritting BG2VOFS: %02x\033[0m\n", value);
	}
	else if (reg == 0x11) {
		printf("\033[0;32mWritting BG3HOFS: %02x\033[0m\n", value);
	}
	else if (reg == 0x12) {
		printf("\033[0;32mWritting BG3VOFS: %02x\033[0m\n", value);
	}
	else if (reg == 0x13) {
		printf("\033[0;32mWritting BG4HOFS: %02x\033[0m\n", value);
	}
	else if (reg == 0x14) {
		printf("\033[0;32mWritting BG4VOFS: %02x\033[0m\n", value);
	}
	else if (reg == 0x15) {
		printf("\033[0;32mWritting VMAIN: %02x\033[0m\n", value);
	}
	else if (reg == 0x16) {
		printf("\033[0;32mWritting VMADDL: %02x\033[0m\n", value);
	}
	else if (reg == 0x17) {
		printf("\033[0;32mWritting VMADDH: %02x\033[0m\n", value);
	}
	else if (reg == 0x18) {
		printf("\033[0;32mWritting VMDATAL: %02x\033[0m\n", value);
	}
	else if (reg == 0x19) {
		printf("\033[0;32mWritting VMDATAH: %02x\033[0m\n", value);
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
