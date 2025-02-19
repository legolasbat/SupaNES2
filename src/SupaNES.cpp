#include "SupaNES.h"

#include <assert.h>
#include <stdio.h>

SupaNES::SupaNES() {
	// TODO: Add APU
	cpu = new CPU(this);
	ppu = new PPU(this);
	cart = new Cartridge(this);
	
	RomLoaded = false;

	memoryBank = 0;
	memoryPage = 0;

	for (int i = 0; i < 0x20000; i++) {
		WRAM[i] = 0;
	}
}

int SupaNES::Clock() {
	int cycles = cpu->Clock();
	for (int i = 0; i < cycles * 2; i++) {
		ppu->Clock();
	}
	return cycles;
}

uint8_t SupaNES::ReadMemory(uint32_t address) {
	memoryBank = (address & 0x00FF0000) >> 16;
	memoryPage = (address & 0x0000FF00) >> 8;

	uint8_t value = 0;

	if (memoryBank < 0x40) {
		value = ReadMemoryQ1(address);
	}
	else if (memoryBank < 0x80) {
		//TODO: Read Q2
		assert(false && "Reading Q2" && address);
	}
	else if (memoryBank < 0xC0) {
		//TODO: Read Q3
		assert(false && "Reading Q3" && address);
	}
	else {
		//TODO: Read Q4
		assert(false && "Reading Q4" && address);
	}

	return value;
}

void SupaNES::WriteMemory(uint32_t address, uint8_t value) {
	memoryBank = (address & 0x00FF0000) >> 16;
	memoryPage = (address & 0x0000FF00) >> 8;

	if (memoryBank < 0x40) {
		WriteMemoryQ1(address, value);
	}
	else if (memoryBank < 0x80) {
		//TODO: Write Q2
		printf("%06X\n", address);
		assert(false && "Writting Q2");
	}
	else if (memoryBank < 0xC0) {
		//TODO: Write Q3
		printf("%06X\n", address);
		assert(false && "Writting Q3");
	}
	else {
		//TODO: Write Q4
		printf("%06X\n", address);
		assert(false && "Writting Q4");
	}
}

uint8_t SupaNES::ReadMemoryQ1(uint32_t address) {
	uint8_t value = 0;

	// WRAM
	if (memoryPage < 0x20) {
		value = WRAM[address];
	}
	else if (memoryPage == 0x21) {
		//TODO: Make PPU, APU and WRAM
		printf("%06X\n", address);
		assert(false && "Reading Q1 Page 21");
	}
	else if (memoryPage == 0x40) {
		// Reading Joypad register
		printf("%06X\n", address);
		assert(false && "Reading Q1 Page 40");
	}
	else if (memoryPage == 0x42 || memoryPage == 0x43) {
		value = cpu->ReadCPU(address);
	}
	else if (memoryPage >= 0x80) {
		value = cart->ReadRom(address);
	}
	return value;
}


void SupaNES::WriteMemoryQ1(uint32_t address, uint8_t value) {
	// WRAM
	if (memoryPage < 0x20) {
		WRAM[address] = value;
	}
	else if (memoryPage == 0x21) {
		//TODO: Make PPU, APU and WRAM
		// PPU
		if ((address & 0xFF) <= 0x3F) {
			ppu->WritePPU(address, value);
		}
		// APU
		else if ((address & 0xFF) <= 0x43) {
			assert(false && "Writting APU");
		}
		// WRAM
		else if ((address & 0xFF) == 0x80) {	// Write/Read
			WRAM[WRAMAddress++] = value;
		}
		else if ((address & 0xFF) == 0x81) {	// Low Address
			WRAMAddress &= 0x1FF00;
			WRAMAddress |= value;
		}
		else if ((address & 0xFF) == 0x82) {	// Page Address
			WRAMAddress &= 0x100FF;
			WRAMAddress |= value << 8;
		}
		else if ((address & 0xFF) == 0x83) {	// Bank Address
			WRAMAddress &= 0xFFFF;
			WRAMAddress |= value << 16;
		} else {
			printf("Page 21: %06X\n", address);
		}
	}
	else if (memoryPage == 0x40) {
		// Writting Joypad register
		printf("Page 40: %06X\n", address);
	}
	// CPU
	else if (memoryPage == 0x42 || memoryPage == 0x43) {
		cpu->WriteCPU(address, value);
	}
	else if (memoryPage >= 0x80) {
		//TODO: Writting cartridge
		printf("%06X\n", address);
		assert(false && "Writting Q1 Cart");
	}
}

bool SupaNES::InsertROM(const char* filepath) {
	uint16_t excVectors[16];
	if (!cart->LoadROM(filepath, excVectors)) {
		return false;
	}

	cpu->SetExcVectors(excVectors);
	
	RomLoaded = true;

	return true;
}
