#pragma once

#include <stdint.h>

class SupaNES;

class PPU {
	public:
		SupaNES* memory;
		
		PPU(SupaNES* supaNes);

		void Clock();

		void WritePPU(uint32_t address, uint8_t value);

	private:
};
