#pragma once

#include <cstdint>

class SupaNES;

class APU {
	public:
		SupaNES* memory;

		APU(SupaNES* supaNes);

		int Clock();

		void Debug();

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

		int cycles = 0;
		uint8_t opcode;

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

		// Timers
		// Timers 0 and 1 ticks at 128 clock cycles (~8000hz) and Timer 2 ticks at 16 clock cycles (~64000hz)
		uint8_t timer0 = 0;
		uint8_t timer0Ticks = 0;
		uint8_t timer1 = 0;
		uint8_t timer1Ticks = 0;
		uint8_t timer2 = 0;
		uint8_t timer2Ticks = 0;

		void CheckTimers();

		uint8_t ReadAPUMemory(uint16_t address);
		void WriteAPUMemory(uint16_t address, uint8_t value);

		// Read and Write IO APU to CPU
		uint8_t ReadCPUPorts(uint8_t address);
		void WriteAPUPorts(uint8_t address, uint8_t value);

		void SetFlag(uint8_t flag, bool state);

		void CheckNFlag(uint8_t value);
		void CheckZFlag(uint8_t value);

		enum class AddMode {
			Implied,
			Immediate,
			ZeroPage,
			ZeroPageIndexX,
			ZeroPageIndexY,
			ZeroPageX,
			ZeroPageIncX,
			ZeroPageY,
			Absolute,
			AbsoluteX,
			AbsoluteY,
			IndirectX,
			IndirectY
		};

		struct Instruction {
			char name[5];
			uint8_t (APU::*execute)(void);
			AddMode addMode;
			uint8_t cycles;
		};
		Instruction opTable[0x100];

		uint8_t fetched;
		uint16_t fetchAddress;

		uint8_t PullStack();
		void PushStack(uint8_t value);

		void FetchValue(AddMode addMode);
		void WriteValue(AddMode addMode, uint8_t value, bool read = false);

		void IMM();
		void ZeroPage();
		void ZeroPageX();
		void ZeroPageIncX();
		void ZeroPageIndexX();
		void ZeroPageIndexY();
		void ZeroPageY();
		void Absolute();
		void AbsoluteX();
		void AbsoluteY();
		void IndirectX();
		void IndirectY();

		uint8_t ADCA();
		uint8_t ADCW();
		uint8_t ADC();
		uint8_t ANDA();
		uint8_t AND();
		uint8_t ANDC();
		uint8_t ASL();
		uint8_t BBC();
		uint8_t BBS();
		uint8_t BCC();
		uint8_t BCS();
		uint8_t BEQ();
		uint8_t BMI();
		uint8_t BNE();
		uint8_t BPL();
		uint8_t BRA();
		uint8_t CBNE();
		uint8_t CLC();
		uint8_t CLP();
		uint8_t CLR();
		uint8_t TCLR();
		uint8_t CLV();
		uint8_t CMP();
		uint8_t CPA();
		uint8_t CPW();
		uint8_t CPX();
		uint8_t CPY();
		uint8_t DBNZ();
		uint8_t DEC();
		uint8_t DEW();
		uint8_t DEX();
		uint8_t DEY();
		uint8_t EORA();
		uint8_t INC();
		uint8_t INW();
		uint8_t INX();
		uint8_t INY();
		uint8_t JMP();
		uint8_t JSR();
		uint8_t LDA();
		uint8_t LDX();
		uint8_t LDY();
		uint8_t LDW();
		uint8_t LSR();
		uint8_t MOV();
		uint8_t MUL();
		uint8_t ORA();
		uint8_t PHA();
		uint8_t PHP();
		uint8_t PHX();
		uint8_t PHY();
		uint8_t PLA();
		uint8_t PLP();
		uint8_t PLX();
		uint8_t PLY();
		uint8_t RTS();
		uint8_t SEC();
		uint8_t SEP();
		uint8_t SET();
		uint8_t TSET();
		uint8_t STA();
		uint8_t STW();
		uint8_t STX();
		uint8_t STY();
		uint8_t TAX();
		uint8_t TAY();
		uint8_t TXA();
		uint8_t TXS();
		uint8_t TYA();
		uint8_t XCN();

		uint8_t XXX();
};
