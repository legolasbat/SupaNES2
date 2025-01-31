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

		bool LoadROM(const char* filepath);

	private:
		bool CheckCartridgeType(const uint8_t* cartType);
		bool CheckRegion(const uint8_t* region);
};
