#pragma once

#include <stdint.h>

#define NFlag 0x80	// Negative flag
#define VFlag 0x40	// Overflow flag
#define MFlag 0x20	// Accumulator size flag (0=16bit, 1=8bit)
#define XFlag 0x10	// Index size flag. Break flag in emulation mode only
#define DFlag 0x08	// Unused. Decimal flag
#define IFlag 0x04	// IRQ Disable flag
#define ZFlag 0x02	// Zero flag
#define CFlag 0x01	// Carry flag

class SupaNES;

class CPU {
	public:
		SupaNES* memory;

		// Internal registers
		struct Registers{
			// Program Counter
			uint16_t PC;

			union HLReg {
				uint16_t HL;
				uint8_t L;
			};

			// Accumulator
			HLReg A;

			// Index
			HLReg X, Y;

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
		
		uint8_t ReadCPU(uint32_t address);
		void WriteCPU(uint32_t address, uint8_t value);

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

		uint8_t ReadMemory(uint32_t address);
		void WriteMemory(uint32_t address, uint8_t value);
		
		enum class AddMode {
			Implied,
			Immediate,
			Absolute,
			AbsoluteIndirect,
			AbsoluteIndirectLong,
			AbsoluteLong,
			AbsoluteLongIndexedX,
			AbsoluteIndexedX,
			AbsoluteIndexedIndirectX,
			AbsoluteIndexedY,
			DirectPage,
			DirectPageIndexedX,
			DirectPageIndexedY,
			DirectPageIndirect,
			DirectPageIndirectX,
			DirectPageIndirectIndexedY,
			DirectPageIndirectLong,
			DirectPageIndirectLongIndexedY,
			StackRelative,
			StackRelativeIndirectIndexedY
		};
		
		struct Instruction {
			char name[4];
			uint8_t (CPU::*execute)(void);
			AddMode addMode;
			uint8_t cycles;
		};
		Instruction opTable[0x100];
		
		uint8_t opcode;
		uint16_t fetched;
		uint32_t fetchAddress;

		bool movActive = false;
		uint8_t srcBank;

		void SetFlag(uint8_t flag, bool state);
		
		void CheckNFlag8Bits(uint8_t value);
		void CheckZFlag8Bits(uint8_t value);
		void CheckNFlag16Bits(uint16_t value);
		void CheckZFlag16Bits(uint16_t value);

		uint8_t PullStack();
		void PushStack(uint8_t value);

		// Addressing Modes
		uint8_t FetchValue(AddMode addMode, bool is16Bits);
		uint8_t WriteValue(AddMode addMode, uint16_t value, bool is16Bits, bool read = false);

		uint8_t IMP(bool is16Bits);
		uint8_t IMM(bool is16Bits);
		uint8_t ABS();
		uint8_t ABSLong();
		uint8_t ABSLongIndexX();
		uint8_t ABSIndexX();
		uint8_t ABSIndexY();
		uint8_t DirectPage();
		uint8_t DirectPageIndexedX();
		uint8_t DirectPageIndexedY();
		uint8_t DirectPageIndirect();
		uint8_t DirectPageIndirectX();
		uint8_t DirectPageIndirectIndexedY();
		uint8_t DirectPageIndirectLong();
		uint8_t DirectPageIndirectLongIndexedY();
		uint8_t StackRel();
		uint8_t StackRelativeIndirectIndexedY();

		// OPCODES
		uint8_t ADC();
		uint8_t AND();
		uint8_t ASL();
		uint8_t BCC();
		uint8_t BCS();
		uint8_t BEQ();
		uint8_t BIT();
		uint8_t BMI();
		uint8_t BNE();
		uint8_t BPL();
		uint8_t BRA();
		uint8_t BRK();
		uint8_t BRL();
		uint8_t BVC();
		uint8_t BVS();
		uint8_t CLC();
		uint8_t CLD();
		uint8_t CLI();
		uint8_t CLV();
		uint8_t CMP();
		uint8_t COP();
		uint8_t CPX();
		uint8_t CPY();
		uint8_t DEC();
		uint8_t DEX();
		uint8_t DEY();
		uint8_t EOR();
		uint8_t INC();
		uint8_t INX();
		uint8_t INY();
		uint8_t JML();
		uint8_t JMP();
		uint8_t JSL();
		uint8_t JSR();
		uint8_t LDA();
		uint8_t LDX();
		uint8_t LDY();
		uint8_t LSR();
		uint8_t MVN();
		uint8_t MVP();
		uint8_t NOP();
		uint8_t ORA();
		uint8_t PEA();
		uint8_t PEI();
		uint8_t PER();
		uint8_t PHA();
		uint8_t PHB();
		uint8_t PHD();
		uint8_t PHK();
		uint8_t PHP();
		uint8_t PHX();
		uint8_t PHY();
		uint8_t PLA();
		uint8_t PLB();
		uint8_t PLD();
		uint8_t PLP();
		uint8_t PLX();
		uint8_t PLY();
		uint8_t REP();
		uint8_t ROL();
		uint8_t ROR();
		uint8_t RTI();
		uint8_t RTL();
		uint8_t RTS();
		uint8_t SBC();
		uint8_t SEC();
		uint8_t SED();
		uint8_t SEI();
		uint8_t SEP();
		uint8_t STA();
		uint8_t STP();
		uint8_t STX();
		uint8_t STY();
		uint8_t STZ();
		uint8_t TAX();
		uint8_t TAY();
		uint8_t TCD();
		uint8_t TCS();
		uint8_t TDC();
		uint8_t TSC();
		uint8_t TSX();
		uint8_t TXA();
		uint8_t TXY();
		uint8_t TYA();
		uint8_t TYX();
		uint8_t TRB();
		uint8_t TSB();
		uint8_t TXS();
		uint8_t WAI();
		uint8_t WDM();
		uint8_t XBA();
		uint8_t XCE();
		uint8_t XXX();
};
