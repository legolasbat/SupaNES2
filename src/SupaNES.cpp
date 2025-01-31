#include "SupaNES.h"

SupaNES::SupaNES() {
	// TODO: Create pointers to elements
	cpu = new CPU(this);
	cart = new Cartridge(this);
	
	RomLoaded = false;

	memoryBank = 0;
	memoryPage = 0;
}

int SupaNES::Clock() {
	cpu->Clock();
	return 0;
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
	}
	else if (memoryBank < 0xC0) {
		//TODO: Read Q3
	}
	else {
		//TODO: Read Q4
	}

	return value;
}

uint8_t SupaNES::ReadMemoryQ1(uint32_t address) {
	uint8_t value = 0;

	// WRAM
	if (memoryPage < 0x20) {
		//TODO: Make WRAM
	}
	else if (memoryPage == 0x21) {
		//TODO: Make PPU, APU and WRAM
	}
	else if (memoryPage == 0x40) {
		// Reading Joypad register
	}
	else if (memoryPage == 0x42 || memoryPage == 0x43) {
		//TODO: Read CPU
	}
	else if (memoryPage >= 0x80) {
		value = cart->ReadRom(address);
	}
	return value;
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
