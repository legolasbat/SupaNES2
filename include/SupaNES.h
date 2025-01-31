#pragma once

#include "CPU.h"
#include "Cartridge.h"

class SupaNES {
	public:
		CPU* cpu;
		// Add PPU
		// Add APU
		Cartridge* cart;
		
		bool RomLoaded;

		SupaNES();

		int Clock();

		uint8_t ReadMemory(uint32_t address);

		bool InsertROM(const char* filepath);

	private:
		uint8_t memoryBank;
		uint8_t memoryPage;

		uint8_t ReadMemoryQ1(uint32_t address);
};
