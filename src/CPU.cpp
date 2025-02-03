#include "CPU.h"

#include "SupaNES.h"

#include <stdio.h>

CPU::CPU(SupaNES* supaNes) {
	memory = supaNes;

	emulationMode = true;

	regs.PC = 0x8000;

	regs.A = 0;
	regs.X = 0;
	regs.Y = 0;

	regs.SP = 0x01FF;

	regs.DP = 0;

	regs.PB = 0;

	regs.DB = 0;

	regs.P = 0x34;

	opTable[0x00] = {"BRK", &CPU::NOP, &CPU::IMP, 7};
	opTable[0x01] = {"ORA", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x02] = {"COP", &CPU::NOP, &CPU::IMP, 7};
	opTable[0x03] = {"ORA", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x04] = {"TSB", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x05] = {"ORA", &CPU::NOP, &CPU::IMP, 3};
	opTable[0x06] = {"ASL", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x07] = {"ORA", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x08] = {"PHP", &CPU::NOP, &CPU::IMP, 3};
	opTable[0x09] = {"ORA", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x0A] = {"ASL", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x0B] = {"PHD", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x0C] = {"TSB", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x0D] = {"ORA", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x0E] = {"ASL", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x0F] = {"ORA", &CPU::NOP, &CPU::IMP, 5};

	opTable[0x10] = {"BPL", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x11] = {"ORA", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x12] = {"ORA", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x13] = {"ORA", &CPU::NOP, &CPU::IMP, 7};
	opTable[0x14] = {"TRB", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x15] = {"ORA", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x16] = {"ASL", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x17] = {"ORA", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x18] = {"CLC", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x19] = {"ORA", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x1A] = {"INC", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x1B] = {"TCS", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x1C] = {"TRB", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x1D] = {"ORA", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x1E] = {"ASL", &CPU::NOP, &CPU::IMP, 7};
	opTable[0x1F] = {"ORA", &CPU::NOP, &CPU::IMP, 5};

	opTable[0x20] = {"JSR", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x21] = {"AND", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x22] = {"JSL", &CPU::NOP, &CPU::IMP, 8};
	opTable[0x23] = {"AND", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x24] = {"BIT", &CPU::NOP, &CPU::IMP, 3};
	opTable[0x25] = {"AND", &CPU::NOP, &CPU::IMP, 3};
	opTable[0x26] = {"ROL", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x27] = {"AND", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x28] = {"PLP", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x29] = {"AND", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x2A] = {"ROL", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x2B] = {"PLD", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x2C] = {"BIT", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x2D] = {"AND", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x2E] = {"ROL", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x2F] = {"AND", &CPU::NOP, &CPU::IMP, 5};

	opTable[0x30] = {"BMI", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x31] = {"AND", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x32] = {"AND", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x33] = {"AND", &CPU::NOP, &CPU::IMP, 7};
	opTable[0x34] = {"BIT", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x35] = {"AND", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x36] = {"ROL", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x37] = {"AND", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x38] = {"SEC", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x39] = {"AND", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x3A] = {"DEC", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x3B] = {"TSC", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x3C] = {"BIT", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x3D] = {"AND", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x3E] = {"ROL", &CPU::NOP, &CPU::IMP, 7};
	opTable[0x3F] = {"AND", &CPU::NOP, &CPU::IMP, 5};

	opTable[0x40] = {"RTI", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x41] = {"EOR", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x42] = {"WDM", &CPU::NOP, &CPU::IMP, 2};	// Unused
	opTable[0x43] = {"EOR", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x44] = {"MVP", &CPU::NOP, &CPU::IMP, 0};
	opTable[0x45] = {"EOR", &CPU::NOP, &CPU::IMP, 3};
	opTable[0x46] = {"LSR", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x47] = {"EOR", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x48] = {"PHA", &CPU::NOP, &CPU::IMP, 3};
	opTable[0x49] = {"EOR", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x4A] = {"LSR", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x4B] = {"PHK", &CPU::NOP, &CPU::IMP, 3};
	opTable[0x4C] = {"JMP", &CPU::NOP, &CPU::IMP, 3};
	opTable[0x4D] = {"EOR", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x4E] = {"LSR", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x4F] = {"EOR", &CPU::NOP, &CPU::IMP, 5};

	opTable[0x50] = {"BVC", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x51] = {"EOR", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x52] = {"EOR", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x53] = {"EOR", &CPU::NOP, &CPU::IMP, 7};
	opTable[0x54] = {"MVN", &CPU::NOP, &CPU::IMP, 0};
	opTable[0x55] = {"EOR", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x56] = {"LSR", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x57] = {"EOR", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x58] = {"CLI", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x59] = {"EOR", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x5A] = {"PHY", &CPU::NOP, &CPU::IMP, 3};
	opTable[0x5B] = {"TCD", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x5C] = {"JML", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x5D] = {"EOR", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x5E] = {"LSR", &CPU::NOP, &CPU::IMP, 7};
	opTable[0x5F] = {"EOR", &CPU::NOP, &CPU::IMP, 5};

	opTable[0x60] = {"RTS", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x61] = {"ADC", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x62] = {"PER", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x63] = {"ADC", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x64] = {"STZ", &CPU::NOP, &CPU::IMP, 3};
	opTable[0x65] = {"ADC", &CPU::NOP, &CPU::IMP, 3};
	opTable[0x66] = {"ROR", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x67] = {"ADC", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x68] = {"PLA", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x69] = {"ADC", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x6A] = {"ROR", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x6B] = {"RTL", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x6C] = {"JMP", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x6D] = {"ADC", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x6E] = {"ROR", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x6F] = {"ADC", &CPU::NOP, &CPU::IMP, 5};

	opTable[0x70] = {"BVS", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x71] = {"ADC", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x72] = {"ADC", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x73] = {"ADC", &CPU::NOP, &CPU::IMP, 7};
	opTable[0x74] = {"STZ", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x75] = {"ADC", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x76] = {"ROR", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x77] = {"ADC", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x78] = {"SEI", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x79] = {"ADC", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x7A] = {"PLY", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x7B] = {"TDC", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x7C] = {"JMP", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x7D] = {"ADC", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x7E] = {"ROR", &CPU::NOP, &CPU::IMP, 7};
	opTable[0x7F] = {"ADC", &CPU::NOP, &CPU::IMP, 5};

	opTable[0x80] = {"BRA", &CPU::NOP, &CPU::IMP, 3};
	opTable[0x81] = {"STA", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x82] = {"BRL", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x83] = {"STA", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x84] = {"STY", &CPU::NOP, &CPU::IMP, 3};
	opTable[0x85] = {"STA", &CPU::NOP, &CPU::IMP, 3};
	opTable[0x86] = {"STX", &CPU::NOP, &CPU::IMP, 3};
	opTable[0x87] = {"STA", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x88] = {"DEY", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x89] = {"BIT", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x8A] = {"TXA", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x8B] = {"PHB", &CPU::NOP, &CPU::IMP, 3};
	opTable[0x8C] = {"STY", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x8D] = {"STA", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x8E] = {"STX", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x8F] = {"STA", &CPU::NOP, &CPU::IMP, 5};

	opTable[0x90] = {"BCC", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x91] = {"STA", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x92] = {"STA", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x93] = {"STA", &CPU::NOP, &CPU::IMP, 7};
	opTable[0x94] = {"STY", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x95] = {"STA", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x96] = {"STX", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x97] = {"STA", &CPU::NOP, &CPU::IMP, 6};
	opTable[0x98] = {"TYA", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x99] = {"STA", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x9A] = {"TXS", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x9B] = {"TXY", &CPU::NOP, &CPU::IMP, 2};
	opTable[0x9C] = {"STZ", &CPU::NOP, &CPU::IMP, 4};
	opTable[0x9D] = {"STA", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x9E] = {"STZ", &CPU::NOP, &CPU::IMP, 5};
	opTable[0x9F] = {"STA", &CPU::NOP, &CPU::IMP, 5};

	opTable[0xA0] = {"LDY", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xA1] = {"LDA", &CPU::NOP, &CPU::IMP, 6};
	opTable[0xA2] = {"LDX", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xA3] = {"LDA", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xA4] = {"LDY", &CPU::NOP, &CPU::IMP, 3};
	opTable[0xA5] = {"LDA", &CPU::NOP, &CPU::IMP, 3};
	opTable[0xA6] = {"LDX", &CPU::NOP, &CPU::IMP, 3};
	opTable[0xA7] = {"LDA", &CPU::NOP, &CPU::IMP, 6};
	opTable[0xA8] = {"TAY", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xA9] = {"LDA", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xAA] = {"TAX", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xAB] = {"PLB", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xAC] = {"LDY", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xAD] = {"LDA", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xAE] = {"LDX", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xAF] = {"LDA", &CPU::NOP, &CPU::IMP, 5};

	opTable[0xB0] = {"BCC", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xB1] = {"LDA", &CPU::NOP, &CPU::IMP, 5};
	opTable[0xB2] = {"LDA", &CPU::NOP, &CPU::IMP, 5};
	opTable[0xB3] = {"LDA", &CPU::NOP, &CPU::IMP, 7};
	opTable[0xB4] = {"LDY", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xB5] = {"LDA", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xB6] = {"LDX", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xB7] = {"LDA", &CPU::NOP, &CPU::IMP, 6};
	opTable[0xB8] = {"CLV", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xB9] = {"LDA", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xBA] = {"TSX", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xBB] = {"TYX", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xBC] = {"LDY", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xBD] = {"LDA", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xBE] = {"LDX", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xBF] = {"LDA", &CPU::NOP, &CPU::IMP, 5};

	opTable[0xC0] = {"CPY", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xC1] = {"CMP", &CPU::NOP, &CPU::IMP, 6};
	opTable[0xC2] = {"REP", &CPU::NOP, &CPU::IMP, 3};
	opTable[0xC3] = {"CMP", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xC4] = {"CPY", &CPU::NOP, &CPU::IMP, 3};
	opTable[0xC5] = {"CMP", &CPU::NOP, &CPU::IMP, 3};
	opTable[0xC6] = {"DEC", &CPU::NOP, &CPU::IMP, 5};
	opTable[0xC7] = {"CMP", &CPU::NOP, &CPU::IMP, 6};
	opTable[0xC8] = {"INY", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xC9] = {"CMP", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xCA] = {"DEX", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xCB] = {"WAI", &CPU::NOP, &CPU::IMP, 3};
	opTable[0xCC] = {"CPY", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xCD] = {"CMP", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xCE] = {"DEC", &CPU::NOP, &CPU::IMP, 6};
	opTable[0xCF] = {"CMP", &CPU::NOP, &CPU::IMP, 5};

	opTable[0xD0] = {"BNE", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xD1] = {"CMP", &CPU::NOP, &CPU::IMP, 5};
	opTable[0xD2] = {"CMP", &CPU::NOP, &CPU::IMP, 5};
	opTable[0xD3] = {"CMP", &CPU::NOP, &CPU::IMP, 7};
	opTable[0xD4] = {"PEI", &CPU::NOP, &CPU::IMP, 6};
	opTable[0xD5] = {"CMP", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xD6] = {"DEC", &CPU::NOP, &CPU::IMP, 6};
	opTable[0xD7] = {"CMP", &CPU::NOP, &CPU::IMP, 6};
	opTable[0xD8] = {"CLD", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xD9] = {"CMP", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xDA] = {"PHX", &CPU::NOP, &CPU::IMP, 3};
	opTable[0xDB] = {"STP", &CPU::NOP, &CPU::IMP, 3};
	opTable[0xDC] = {"JML", &CPU::NOP, &CPU::IMP, 6};
	opTable[0xDD] = {"CMP", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xDE] = {"DEC", &CPU::NOP, &CPU::IMP, 7};
	opTable[0xDF] = {"CMP", &CPU::NOP, &CPU::IMP, 5};

	opTable[0xE0] = {"CPX", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xE1] = {"SBC", &CPU::NOP, &CPU::IMP, 6};
	opTable[0xE2] = {"SEP", &CPU::NOP, &CPU::IMP, 3};
	opTable[0xE3] = {"SBC", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xE4] = {"CPX", &CPU::NOP, &CPU::IMP, 3};
	opTable[0xE5] = {"SBC", &CPU::NOP, &CPU::IMP, 3};
	opTable[0xE6] = {"INC", &CPU::NOP, &CPU::IMP, 5};
	opTable[0xE7] = {"SBC", &CPU::NOP, &CPU::IMP, 6};
	opTable[0xE8] = {"INX", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xE9] = {"SBC", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xEA] = {"NOP", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xEB] = {"XBA", &CPU::NOP, &CPU::IMP, 3};
	opTable[0xEC] = {"CPX", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xED] = {"SBC", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xEE] = {"INC", &CPU::NOP, &CPU::IMP, 6};
	opTable[0xEF] = {"SBC", &CPU::NOP, &CPU::IMP, 5};

	opTable[0xF0] = {"BEQ", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xF1] = {"SBC", &CPU::NOP, &CPU::IMP, 5};
	opTable[0xF2] = {"SBC", &CPU::NOP, &CPU::IMP, 5};
	opTable[0xF3] = {"SBC", &CPU::NOP, &CPU::IMP, 7};
	opTable[0xF4] = {"PEA", &CPU::NOP, &CPU::IMP, 5};
	opTable[0xF5] = {"SBC", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xF6] = {"INC", &CPU::NOP, &CPU::IMP, 6};
	opTable[0xF7] = {"SBC", &CPU::NOP, &CPU::IMP, 6};
	opTable[0xF8] = {"SED", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xF9] = {"SBC", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xFA] = {"PLX", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xFB] = {"XCE", &CPU::NOP, &CPU::IMP, 2};
	opTable[0xFC] = {"JSR", &CPU::NOP, &CPU::IMP, 8};
	opTable[0xFD] = {"SBC", &CPU::NOP, &CPU::IMP, 4};
	opTable[0xFE] = {"INC", &CPU::NOP, &CPU::IMP, 7};
	opTable[0xFF] = {"SBC", &CPU::NOP, &CPU::IMP, 5};
}

int CPU::Clock() {
	uint8_t opcode = ReadMemory((regs.PB << 16) | regs.PC, true);

	printf("PC: 0x%02x%04x OP: %s\n", regs.PB, regs.PC, opTable[opcode].name);
	Debug();

	uint8_t cycles = opTable[opcode].cycles;
	cycles += (this->*opTable[opcode].addressMode)();
	cycles += (this->*opTable[opcode].execute)();

	regs.PC++;

	return cycles;
}

uint8_t CPU::ReadMemory(uint32_t address, bool isLong) {
	if (isLong) {
		return memory->ReadMemory(address);
	}
	return memory->ReadMemory((regs.DB << 16) | address);
}

void CPU::SetExcVectors(uint16_t vectors[16]) {
	excVectors.COPVector = vectors[2];
	excVectors.BRKVector = vectors[3];
	excVectors.NMIVector = vectors[5];
	excVectors.IRQVector = vectors[7];

	excVectors.ResetVector = vectors[14];

	regs.PC = excVectors.ResetVector;
}

void CPU::Debug() {
	printf("%02x%04x", regs.PB, regs.PC);
	printf(" A:%04x", regs.A);
	printf(" X:%04x", regs.X);
	printf(" Y:%04x", regs.Y);
	printf(" S:%04x", regs.SP);
	printf(" D:%04x", regs.DP);
	printf(" DB:%04x ", regs.DB);
	
	if (regs.P & NFlag) {
		printf("N");
	} else {
		printf("n");
	}
	if (regs.P & VFlag) {
		printf("V");
	} else {
		printf("v");
	}
	if (emulationMode) {
		printf("1B");
	} else {
		if (regs.P & MFlag) {
			printf("M");
		} else {
			printf("m");
		}
		if (regs.P & XFlag) {
			printf("X");
		} else {
			printf("x");
		}
	}
	if (regs.P & DFlag) {
		printf("D");
	} else {
		printf("d");
	}
	if (regs.P & IFlag) {
		printf("I");
	} else {
		printf("i");
	}
	if (regs.P & ZFlag) {
		printf("Z");
	} else {
		printf("z");
	}
	if (regs.P & CFlag) {
		printf("C");
	} else {
		printf("c");
	}

	printf("\n");
}

uint8_t CPU::IMP() {
	return 0;
}

uint8_t CPU::NOP() {
	return 0;
}
