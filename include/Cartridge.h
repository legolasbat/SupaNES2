#pragma once

#include <stdint.h>

class SupaNES;

class Cartridge {
	public:
		SupaNES* memory;

		uint8_t* ROM;
		int ROMSize;

		Cartridge(SupaNES* supaNes);
		~Cartridge();

		bool LoadROM(const char* filepath, uint16_t* excVectors);

		uint8_t ReadRom(uint32_t address);
		void WriteRom(uint32_t address, uint8_t value);

		uint8_t ReadRam(uint32_t address);
		void WriteRam(uint32_t address, uint8_t value);

	private:
		bool CheckCartridgeType(const uint8_t* cartType);
		bool CheckRegion(const uint8_t* region);
};
