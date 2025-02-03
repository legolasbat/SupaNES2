#pragma once

#include <stdint.h>

#define NFlag 0x80
#define VFlag 0x40
#define MFlag 0x20
#define XFlag 0x10	// Break flag in emulation mode only
#define DFlag 0x08	// Unused
#define IFlag 0x04
#define ZFlag 0x02
#define CFlag 0x01

class SupaNES;

class CPU {
	public:
		SupaNES* memory;

		// Internal registers
		struct Registers{
			// Program Counter
			uint16_t PC;

			// Accumulator
			uint16_t A;

			// Index
			uint16_t X, Y;

			// Stack Pointer
			uint16_t SP;

			// Direct Page;
			uint16_t DP;

			// Program Bank
			uint8_t PB;

			// Data Bank
			uint8_t DB;

			// Processor Status
			uint8_t P;
		} regs;

		CPU(SupaNES* supaNes);
		
		int Clock();

		void SetExcVectors(uint16_t vectors[16]);

		void Debug();

	private:
		
		bool emulationMode;

		struct ExcVectors{
			uint16_t COPVector;
			uint16_t BRKVector;
			uint16_t NMIVector;
			uint16_t IRQVector;

			uint16_t ResetVector;
		} excVectors;

		uint8_t ReadMemory(uint32_t address, bool isLong);

		struct Instruction {
			char name[4];
			uint8_t (CPU::*execute)(void);
			uint8_t (CPU::*addressMode)(void);
			uint8_t cycles;
		};
		Instruction opTable[0x100];

		// Addressing Modes
		uint8_t IMP();

		// OPCODES
		uint8_t NOP();
};
