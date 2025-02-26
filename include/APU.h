#pragma once

#include <cstdint>

class SupaNES;

class APU {
	public:
		SupaNES* memory;

		APU(SupaNES* supaNes);

		// Internal registers

		// Program Counter
		uint16_t PC;

		// Accumulator
		uint8_t A;

		// Index
		uint8_t X, Y;

		// Stack Pointer
		uint8_t SP;

		// Program Status Word
		uint8_t PSW;

		// Read and Write IO CPU to APU
		uint8_t ReadAPUPorts(uint8_t address);
		void WriteCPUPorts(uint8_t address, uint8_t value);

	private:

		// Page 0: 0x0000 - 0x00EF
		uint8_t page0[0xF0];

		// Registers: 0x00F0 - 0x00FF
		uint8_t registers0[0x4];
		uint8_t CPUPorts[0x4];
		uint8_t APUPorts[0x4];
		uint8_t registers1[0x8];

		// Page 1: 0x100 - 0x01FF
		uint8_t page1[0x100];

		// Memory: 0x0200 - 0xFFFF
		uint8_t RAM[0xFE00];

		// Read and Write IO APU to CPU
		uint8_t ReadCPUPorts(uint8_t address);
		void WriteAPUPorts(uint8_t address, uint8_t value);
};
