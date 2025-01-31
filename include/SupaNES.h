#pragma once

#include "Cartridge.h"

class SupaNES {
	public:
		// Add CPU
		// Add PPU
		// Add APU
		Cartridge* cart;
		
		SupaNES();
		~SupaNES();

		bool InsertROM(const char* filepath);
};
