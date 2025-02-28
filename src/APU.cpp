#include "APU.h"

#include <cassert>
#include <cstdint>
#include <cstdio>

#define NFlag 0x80	// Negative flag
#define VFlag 0x40	// Overflow flag
#define PFlag 0x20	// Zero Page Location (0=00xx, 1=01xx)
#define BFlag 0x10	// (0=Reset, 1=BRK opcode; set after BRK opcode)
#define HFlag 0x08	// Half-carry
#define IFlag 0x04	// Interrupt Enable flag (no function in SNES APU)
#define ZFlag 0x02	// Zero flag
#define CFlag 0x01	// Carry flag

APU::APU(SupaNES* supaNes) {
	memory = supaNes;

	PC = 0xFFC0;	// Point to boot ROM

	A = 0;
	X = 0;
	Y = 0;

	uint8_t bootROM[64] = {
		0xCD, 0xEF, 0xBD, 0xE8, 0x00, 0xC6, 0x1D, 0xD0,
		0xFC, 0x8F, 0xAA, 0xF4, 0x8F, 0xBB, 0xF5, 0x78,
		0xCC, 0xF4, 0xD0, 0xFB, 0x2F, 0x19, 0xEB, 0xF4,
		0xD0, 0xFC, 0x7E, 0xF4, 0xD0, 0x0B, 0xE4, 0xF5,
		0xCB, 0xF4, 0xD7, 0x00, 0xFC, 0xD0, 0xF3, 0xAB,
		0x01, 0x10, 0xEF, 0x7E, 0xF4, 0x10, 0xEB, 0xBA,
		0xF6, 0xDA, 0x00, 0xBA, 0xF4, 0xC4, 0xF4, 0xDD,
		0x5D, 0xD0, 0xDB, 0x1F, 0x00, 0x00, 0xC0, 0xFF
	};

	for (int i = 0; i < 64; i++) {
		RAM[PC - 0x200 + i] = bootROM[i];
	}

	opTable[0x00] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x01] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x02] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x03] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x04] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x05] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x06] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x07] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x08] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x09] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x0A] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x0B] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x0C] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x0D] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x0E] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x0F] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0x10] = {"BPL", &APU::BPL, AddMode::Implied, 2};
	opTable[0x11] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x12] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x13] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x14] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x15] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x16] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x17] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x18] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x19] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x1A] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x1B] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x1C] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x1D] = {"DEX", &APU::DEX, AddMode::Implied, 2};
	opTable[0x1E] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x1F] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0x20] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x21] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x22] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x23] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x24] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x25] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x26] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x27] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x28] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x29] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x2A] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x2B] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x2C] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x2D] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x2E] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x2F] = {"BRA", &APU::BRA, AddMode::Immediate, 4};
	
	opTable[0x30] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x31] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x32] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x33] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x34] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x35] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x36] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x37] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x38] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x39] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x3A] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x3B] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x3C] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x3D] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x3E] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x3F] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0x40] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x41] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x42] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x43] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x44] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x45] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x46] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x47] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x48] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x49] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x4A] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x4B] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x4C] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x4D] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x4E] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x4F] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0x50] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x51] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x52] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x53] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x54] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x55] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x56] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x57] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x58] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x59] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x5A] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x5B] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x5C] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x5D] = {"TAX", &APU::TAX, AddMode::Implied, 2};
	opTable[0x5E] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x5F] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0x60] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x61] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x62] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x63] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x64] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x65] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x66] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x67] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x68] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x69] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x6A] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x6B] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x6C] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x6D] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x6E] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x6F] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0x70] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x71] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x72] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x73] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x74] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x75] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x76] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x77] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x78] = {"CMP", &APU::CMP, AddMode::Immediate, 5};
	opTable[0x79] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x7A] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x7B] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x7C] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x7D] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x7E] = {"CPY", &APU::CPY, AddMode::ZeroPage, 3};
	opTable[0x7F] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0x80] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x81] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x82] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x83] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x84] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x85] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x86] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x87] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x88] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x89] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x8A] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x8B] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x8C] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x8D] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x8E] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x8F] = {"MOV", &APU::MOV, AddMode::Immediate, 5};
	
	opTable[0x90] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x91] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x92] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x93] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x94] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x95] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x96] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x97] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x98] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x99] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x9A] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x9B] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x9C] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x9D] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x9E] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x9F] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0xA0] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xA1] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xA2] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xA3] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xA4] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xA5] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xA6] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xA7] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xA8] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xA9] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xAA] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xAB] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xAC] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xAD] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xAE] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xAF] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0xB0] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xB1] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xB2] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xB3] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xB4] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xB5] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xB6] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xB7] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xB8] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xB9] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xBA] = {"LDW", &APU::LDW, AddMode::Immediate, 5};
	opTable[0xBB] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xBC] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xBD] = {"TXS", &APU::TXS, AddMode::Implied, 2};
	opTable[0xBE] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xBF] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0xC0] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xC1] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xC2] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xC3] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xC4] = {"STA", &APU::STA, AddMode::ZeroPage, 4};
	opTable[0xC5] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xC6] = {"STA", &APU::STA, AddMode::ZeroPageX, 4};
	opTable[0xC7] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xC8] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xC9] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xCA] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xCB] = {"STY", &APU::STY, AddMode::ZeroPage, 4};
	opTable[0xCC] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xCD] = {"LDX", &APU::LDX, AddMode::Immediate, 2};
	opTable[0xCE] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xCF] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0xD0] = {"BNE", &APU::BNE, AddMode::Implied, 2};
	opTable[0xD1] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xD2] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xD3] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xD4] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xD5] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xD6] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xD7] = {"STA", &APU::STA, AddMode::IndirectY, 7};
	opTable[0xD8] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xD9] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xDA] = {"STW", &APU::STW, AddMode::Immediate, 5};
	opTable[0xDB] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xDC] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xDD] = {"TYA", &APU::TYA, AddMode::Implied, 2};
	opTable[0xDE] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xDF] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0xE0] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xE1] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xE2] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xE3] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xE4] = {"LDA", &APU::LDA, AddMode::ZeroPage, 3};
	opTable[0xE5] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xE6] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xE7] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xE8] = {"LDA", &APU::LDA, AddMode::Immediate, 2};
	opTable[0xE9] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xEA] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xEB] = {"LDY", &APU::LDY, AddMode::ZeroPage, 3};
	opTable[0xEC] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xED] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xEE] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xEF] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0xF0] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xF1] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xF2] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xF3] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xF4] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xF5] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xF6] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xF7] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xF8] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xF9] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xFA] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xFB] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xFC] = {"INY", &APU::INY, AddMode::Implied, 2};
	opTable[0xFD] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xFE] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xFF] = {"", &APU::XXX, AddMode::Implied, 2};
}

int APU::Clock() {
	int opcycles = 0;

	if (cycles == 0) {
		opcode = ReadAPUMemory(PC);
		PC++;

		opcycles = opTable[opcode].cycles;
		opcycles += (this->*opTable[opcode].execute)();
		cycles = opcycles;
	}

	cycles--;
	return opcycles;
}

uint8_t APU::ReadAPUMemory(uint16_t address) {
	uint8_t value = 0;

	// Page 0
	if (address <= 0x00EF) {
		value = page0[address];
	}
	// Registers
	else if (address <= 0x00FF) {
		if (address <= 0xF3) {
			assert(address >= 0xF2 && "Reading APU: F0 or F1");
			value = registers0[address- 0xF0];
		}
		else if (address <= 0xF7) {
			value = ReadCPUPorts(address);
		}
		else {
			assert((address <= 0xF9 || address >= 0xFD) && "Reading APU: FA-FC");
			value = registers1[address - 0xF8];
		}
	}
	// Page 1
	else if (address <= 0x01FF) {
		value = page1[address - 0x100];
	}
	// Memory
	else {
		value = RAM[address - 0x200];
	}

	return value;
}

void APU::WriteAPUMemory(uint16_t address, uint8_t value) {
	// Page 0
	if (address <= 0x00EF) {
		page0[address] = value;
	}
	// Registers
	else if (address <= 0x00FF) {
		if (address <= 0xF3) {
			registers0[address- 0xF0] = value;
		}
		else if (address <= 0xF7) {
			WriteAPUPorts(address, value);
		}
		else {
			assert(address <= 0xFC && "Writting APU: FD-FF");
			registers1[address - 0xF8] = value;
		}
	}
	// Page 1
	else if (address <= 0x01FF) {
		page1[address - 0x100] = value;
	}
	// Memory
	else {
		RAM[address - 0x200] = value;
	}
}

// APU IO Ports
uint8_t APU::ReadCPUPorts(uint8_t address) {
	uint8_t index = address & 0x3;

	return CPUPorts[index];
}

void APU::WriteAPUPorts(uint8_t address, uint8_t value) {
	uint8_t index = address & 0x3;

	APUPorts[index] = value;
}

// CPU IO Ports
uint8_t APU::ReadAPUPorts(uint8_t address) {
	uint8_t index = address & 0x3;

	printf("Reading APU Port %02x: %02x\n", address, APUPorts[index]);

	return APUPorts[index];
}

void APU::WriteCPUPorts(uint8_t address, uint8_t value) {
	uint8_t index = address & 0x3;

	CPUPorts[index] = value;
	
	printf("Writting APU Port %02x: %02x\n", address, value);
}

void APU::SetFlag(uint8_t flag, bool state) {
	if (state) {
		PSW |= flag;
	} else {
		PSW &= ~flag;
	}
}

void APU::CheckNFlag(uint8_t value) {
	SetFlag(NFlag, value & 0x80);
}

void APU::CheckZFlag(uint8_t value) {
	SetFlag(ZFlag, !value);
}

uint8_t APU::FetchValue(AddMode addMode) {
	switch (addMode) {
		case AddMode::Immediate:
			return IMM();

		case AddMode::ZeroPage:
		{
			uint16_t address = ReadAPUMemory(PC++);
			if (PSW & PFlag) {
				address += 0x100;
			}
			fetched = ReadAPUMemory(address);

			return 0;

		}

		default:
			printf("%d\n", addMode);
			assert(false);
	}
	return 0;
}

uint8_t APU::WriteValue(AddMode addMode, uint8_t value, bool read) {
	switch (addMode) {
		case AddMode::ZeroPage:
		{
			uint16_t address = ReadAPUMemory(PC++);
			if (PSW & PFlag) {
				address += 0x100;
			}
			WriteAPUMemory(address, value);

			return 0;
		}

		case AddMode::ZeroPageX:
		{
			uint16_t address = X;
			if (PSW & PFlag) {
				address += 0x100;
			}
			WriteAPUMemory(address, value);
			return 0;
		}

		case AddMode::IndirectY:
		{
			uint16_t indAddress = ReadAPUMemory(PC++);
			if (PSW & PFlag) {
				indAddress += 0x100;
			}
			uint16_t address = ReadAPUMemory(indAddress);
			address |= ReadAPUMemory(indAddress + 1) << 8;

			WriteAPUMemory(address + Y, value);

			return 0;
		}

		default:
			printf("%d\n", addMode);
			assert(false);
	}
	return 0;
}

uint8_t APU::IMM() {
	fetched = ReadAPUMemory(PC++);

	return 0;
}

uint8_t APU::BNE() {
	int8_t offset = ReadAPUMemory(PC++);
	if (!(PSW & ZFlag)) {
		PC += offset;
		return 2;
	}

	return 0;
}

uint8_t APU::BPL() {
	int8_t offset = ReadAPUMemory(PC++);
	if (!(PSW & NFlag)) {
		PC += offset;
		return 2;
	}

	return 0;
}

uint8_t APU::BRA() {
	int8_t offset = ReadAPUMemory(PC++);
	PC += offset;

	return 0;
}

uint8_t APU::CMP() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode);
	
	uint16_t address = X;
	if (opcode == 0x78 || opcode == 0x69) {
		address = ReadAPUMemory(PC++);
	}
	if (PSW & PFlag) {
		address += 0x100;
	}

	uint8_t value = ReadAPUMemory(address);
	
	uint8_t result = value - fetched;

	CheckNFlag(result);
	CheckZFlag(result);

	if (result > value) {
		PSW &= ~CFlag;
	} else {
		PSW |= CFlag;
	}

	return extraCycles;
}

uint8_t APU::CPY() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode);
	
	uint8_t result = Y - fetched;

	CheckNFlag(result);
	CheckZFlag(result);

	if (result > Y) {
		PSW &= ~CFlag;
	} else {
		PSW |= CFlag;
	}

	return extraCycles;
}

uint8_t APU::DEX() {
	X--;

	CheckNFlag(X);
	CheckZFlag(X);

	return 0;
}

uint8_t APU::INY() {
	Y++;

	CheckNFlag(Y);
	CheckZFlag(Y);

	return 0;
}

uint8_t APU::LDA() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode);

	A = fetched;

	CheckNFlag(A);
	CheckZFlag(A);

	return extraCycles;
}

uint8_t APU::LDX() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode);

	X = fetched;

	CheckNFlag(X);
	CheckZFlag(X);

	return extraCycles;
}

uint8_t APU::LDY() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode);

	Y = fetched;

	CheckNFlag(Y);
	CheckZFlag(Y);

	return extraCycles;
}

uint8_t APU::LDW() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode);

	uint16_t address = 0;
	if (PSW & PFlag) {
		address += 0x100;
	}

	A = ReadAPUMemory(address + fetched);
	Y = ReadAPUMemory(address + (fetched + 1));

	CheckNFlag(Y);
	if (A == 0 && Y == 0) {
		SetFlag(ZFlag, true);
	} else {
		SetFlag(ZFlag, false);
	}

	return 0;
}

uint8_t APU::MOV() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode);
	
	uint16_t address = ReadAPUMemory(PC++);

	if (PSW & PFlag) {
		address += 0x100;
	}

	WriteAPUMemory(address, fetched);

	return extraCycles;
}

uint8_t APU::STA() {
	uint8_t extraCycles = WriteValue(opTable[opcode].addMode, A);

	return extraCycles;
}

uint8_t APU::STW() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode);

	uint16_t address = 0;
	if (PSW & PFlag) {
		address += 0x100;
	}

	WriteAPUMemory(address + fetched, A);
	WriteAPUMemory(address + (fetched + 1), Y);

	return 0;
}

uint8_t APU::STY() {
	uint8_t extraCycles = WriteValue(opTable[opcode].addMode, Y);

	return extraCycles;
}

uint8_t APU::TAX() {
	X = A;

	CheckNFlag(X);
	CheckZFlag(X);

	return 0;
}

uint8_t APU::TXS() {
	SP = X;

	return 0;
}

uint8_t APU::TYA() {
	A = Y;

	CheckNFlag(A);
	CheckZFlag(A);

	return 0;
}

uint8_t APU::XXX() {
	printf("APU OP: %02x\n", opcode);
	assert(false);
	return 0;
}
