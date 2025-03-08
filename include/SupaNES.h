#pragma once

#include "CPU.h"
#include "PPU.h"
#include "APU.h"
#include "Cartridge.h"
#include <cstdint>

class SupaNES {
	public:
		CPU* cpu;
		PPU* ppu;
		APU* apu;
		Cartridge* cart;
		
		bool RomLoaded;

		SupaNES();

		int Clock();

		uint8_t ReadMemory(uint32_t address);
		void WriteMemory(uint32_t address, uint8_t value);

		bool InsertROM(const char* filepath);

	private:
		uint8_t WRAM[0x20000];

		uint32_t WRAMAddress;

		uint8_t memoryBank;
		uint8_t memoryPage;

		uint8_t ReadMemoryQ1(uint32_t address);
		uint8_t ReadMemoryQ2(uint32_t address);
		uint8_t ReadMemoryQ3(uint32_t address);
		uint8_t ReadMemoryQ4(uint32_t address);

		void WriteMemoryQ1(uint32_t address, uint8_t value);
		void WriteMemoryQ2(uint32_t address, uint8_t value);
		void WriteMemoryQ3(uint32_t address, uint8_t value);
		void WriteMemoryQ4(uint32_t address, uint8_t value);
};
