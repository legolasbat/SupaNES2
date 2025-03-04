#include "APU.h"

#include <cassert>
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

	SP = 0xEF;
	PSW = ZFlag;

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
	opTable[0x02] = {"SET", &APU::SET, AddMode::ZeroPage, 4};
	opTable[0x03] = {"BBS", &APU::BBS, AddMode::ZeroPage, 5};
	opTable[0x04] = {"ORA", &APU::ORA, AddMode::ZeroPage, 3};
	opTable[0x05] = {"ORA", &APU::ORA, AddMode::Absolute, 4};
	opTable[0x06] = {"ORA", &APU::ORA, AddMode::ZeroPageX, 3};
	opTable[0x07] = {"ORA", &APU::ORA, AddMode::IndirectX, 6};
	opTable[0x08] = {"ORA", &APU::ORA, AddMode::Immediate, 2};
	opTable[0x09] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x0A] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x0B] = {"ASL", &APU::ASL, AddMode::ZeroPage, 4};
	opTable[0x0C] = {"ASL", &APU::ASL, AddMode::Absolute, 5};
	opTable[0x0D] = {"PHP", &APU::PHP, AddMode::Implied, 4};
	opTable[0x0E] = {"SET", &APU::TSET, AddMode::Absolute, 6};
	opTable[0x0F] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0x10] = {"BPL", &APU::BPL, AddMode::Implied, 2};
	opTable[0x11] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x12] = {"CLR", &APU::CLR, AddMode::ZeroPage, 4};
	opTable[0x13] = {"BBC", &APU::BBC, AddMode::ZeroPage, 5};
	opTable[0x14] = {"ORA", &APU::ORA, AddMode::ZeroPageIndexX, 4};
	opTable[0x15] = {"ORA", &APU::ORA, AddMode::AbsoluteX, 5};
	opTable[0x16] = {"ORA", &APU::ORA, AddMode::AbsoluteY, 5};
	opTable[0x17] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x18] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x19] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x1A] = {"DEW", &APU::DEW, AddMode::ZeroPage, 6};
	opTable[0x1B] = {"ASL", &APU::ASL, AddMode::ZeroPageIndexX, 5};
	opTable[0x1C] = {"ASL", &APU::ASL, AddMode::Implied, 2};
	opTable[0x1D] = {"DEX", &APU::DEX, AddMode::Implied, 2};
	opTable[0x1E] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x1F] = {"JMP", &APU::JMP, AddMode::Implied, 6};
	
	opTable[0x20] = {"CLP", &APU::CLP, AddMode::Implied, 2};
	opTable[0x21] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x22] = {"SET", &APU::SET, AddMode::ZeroPage, 4};
	opTable[0x23] = {"BBS", &APU::BBS, AddMode::ZeroPage, 5};
	opTable[0x24] = {"AND", &APU::ANDA, AddMode::ZeroPage, 3};
	opTable[0x25] = {"AND", &APU::ANDA, AddMode::Absolute, 4};
	opTable[0x26] = {"AND", &APU::ANDA, AddMode::ZeroPageX, 3};
	opTable[0x27] = {"AND", &APU::ANDA, AddMode::IndirectX, 6};
	opTable[0x28] = {"AND", &APU::ANDA, AddMode::Immediate, 2};
	opTable[0x29] = {"AND", &APU::AND, AddMode::ZeroPage, 6};
	opTable[0x2A] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x2B] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x2C] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x2D] = {"PHA", &APU::PHA, AddMode::Implied, 4};
	opTable[0x2E] = {"CBNE", &APU::CBNE, AddMode::ZeroPage, 5};
	opTable[0x2F] = {"BRA", &APU::BRA, AddMode::Immediate, 4};
	
	opTable[0x30] = {"BMI", &APU::BMI, AddMode::Implied, 2};
	opTable[0x31] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x32] = {"CLR", &APU::CLR, AddMode::ZeroPage, 4};
	opTable[0x33] = {"BBC", &APU::BBC, AddMode::ZeroPage, 5};
	opTable[0x34] = {"AND", &APU::ANDA, AddMode::ZeroPageIndexX, 4};
	opTable[0x35] = {"AND", &APU::ANDA, AddMode::AbsoluteX, 5};
	opTable[0x36] = {"AND", &APU::ANDA, AddMode::AbsoluteY, 5};
	opTable[0x37] = {"AND", &APU::ANDA, AddMode::IndirectY, 6};
	opTable[0x38] = {"AND", &APU::AND, AddMode::ZeroPage, 5};
	opTable[0x39] = {"AND", &APU::AND, AddMode::ZeroPageX, 5};
	opTable[0x3A] = {"INW", &APU::INW, AddMode::ZeroPage, 6};
	opTable[0x3B] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x3C] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x3D] = {"INX", &APU::INX, AddMode::Implied, 2};
	opTable[0x3E] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x3F] = {"JSR", &APU::JSR, AddMode::Implied, 8};
	
	opTable[0x40] = {"SEP", &APU::SEP, AddMode::Implied, 2};
	opTable[0x41] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x42] = {"SET", &APU::SET, AddMode::ZeroPage, 4};
	opTable[0x43] = {"BBS", &APU::BBS, AddMode::ZeroPage, 5};
	opTable[0x44] = {"EORA", &APU::EORA, AddMode::ZeroPage, 3};
	opTable[0x45] = {"EORA", &APU::EORA, AddMode::Absolute, 4};
	opTable[0x46] = {"EORA", &APU::EORA, AddMode::ZeroPageX, 3};
	opTable[0x47] = {"EORA", &APU::EORA, AddMode::IndirectX, 6};
	opTable[0x48] = {"EORA", &APU::EORA, AddMode::Immediate, 2};
	opTable[0x49] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x4A] = {"ANDC", &APU::ANDC, AddMode::Implied, 4};
	opTable[0x4B] = {"LSR", &APU::LSR, AddMode::ZeroPage, 4};
	opTable[0x4C] = {"LSR", &APU::LSR, AddMode::Absolute, 5};
	opTable[0x4D] = {"PHX", &APU::PHX, AddMode::Implied, 4};
	opTable[0x4E] = {"CLR", &APU::TCLR, AddMode::Absolute, 6};
	opTable[0x4F] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0x50] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x51] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x52] = {"CLR", &APU::CLR, AddMode::ZeroPage, 4};
	opTable[0x53] = {"BBC", &APU::BBC, AddMode::ZeroPage, 5};
	opTable[0x54] = {"EORA", &APU::EORA, AddMode::ZeroPageIndexX, 4};
	opTable[0x55] = {"EORA", &APU::EORA, AddMode::AbsoluteX, 5};
	opTable[0x56] = {"EORA", &APU::EORA, AddMode::AbsoluteY, 5};
	opTable[0x57] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x58] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x59] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x5A] = {"CPW", &APU::CPW, AddMode::ZeroPage, 4};
	opTable[0x5B] = {"LSR", &APU::LSR, AddMode::ZeroPageIndexX, 5};
	opTable[0x5C] = {"LSR", &APU::LSR, AddMode::Implied, 2};
	opTable[0x5D] = {"TAX", &APU::TAX, AddMode::Implied, 2};
	opTable[0x5E] = {"CPY", &APU::CPY, AddMode::Absolute, 4};
	opTable[0x5F] = {"JMP", &APU::JMP, AddMode::Implied, 3};
	
	opTable[0x60] = {"CLC", &APU::CLC, AddMode::Implied, 2};
	opTable[0x61] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x62] = {"SET", &APU::SET, AddMode::ZeroPage, 4};
	opTable[0x63] = {"BBS", &APU::BBS, AddMode::ZeroPage, 5};
	opTable[0x64] = {"CMP", &APU::CPA, AddMode::ZeroPage, 3};
	opTable[0x65] = {"CMP", &APU::CPA, AddMode::Absolute, 4};
	opTable[0x66] = {"CMP", &APU::CPA, AddMode::ZeroPageX, 3};
	opTable[0x67] = {"CMP", &APU::CPA, AddMode::IndirectX, 6};
	opTable[0x68] = {"CMP", &APU::CPA, AddMode::Immediate, 2};
	opTable[0x69] = {"CMP", &APU::CMP, AddMode::ZeroPage, 6};
	opTable[0x6A] = {"ANDC", &APU::ANDC, AddMode::Implied, 4};
	opTable[0x6B] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x6C] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x6D] = {"PHY", &APU::PHY, AddMode::Implied, 4};
	opTable[0x6E] = {"DBNZ", &APU::DBNZ, AddMode::ZeroPage, 5};
	opTable[0x6F] = {"RTS", &APU::RTS, AddMode::Implied, 5};
	
	opTable[0x70] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x71] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x72] = {"CLR", &APU::CLR, AddMode::ZeroPage, 4};
	opTable[0x73] = {"BBC", &APU::BBC, AddMode::ZeroPage, 5};
	opTable[0x74] = {"CMP", &APU::CPA, AddMode::ZeroPageIndexX, 4};
	opTable[0x75] = {"CMP", &APU::CPA, AddMode::AbsoluteX, 5};
	opTable[0x76] = {"CMP", &APU::CPA, AddMode::AbsoluteY, 5};
	opTable[0x77] = {"CMP", &APU::CPA, AddMode::IndirectY, 6};
	opTable[0x78] = {"CMP", &APU::CMP, AddMode::Immediate, 5};
	opTable[0x79] = {"CMP", &APU::CMP, AddMode::ZeroPageY, 5};
	opTable[0x7A] = {"ADW", &APU::ADCW, AddMode::ZeroPage, 5};
	opTable[0x7B] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x7C] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x7D] = {"TXA", &APU::TXA, AddMode::Implied, 2};
	opTable[0x7E] = {"CPY", &APU::CPY, AddMode::ZeroPage, 3};
	opTable[0x7F] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0x80] = {"SEC", &APU::SEC, AddMode::Implied, 2};
	opTable[0x81] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x82] = {"SET", &APU::SET, AddMode::ZeroPage, 4};
	opTable[0x83] = {"BBS", &APU::BBS, AddMode::ZeroPage, 5};
	opTable[0x84] = {"ADC", &APU::ADCA, AddMode::ZeroPage, 3};
	opTable[0x85] = {"ADC", &APU::ADCA, AddMode::Absolute, 4};
	opTable[0x86] = {"ADC", &APU::ADCA, AddMode::ZeroPageX, 3};
	opTable[0x87] = {"ADC", &APU::ADCA, AddMode::IndirectX, 6};
	opTable[0x88] = {"ADC", &APU::ADCA, AddMode::Immediate, 2};
	opTable[0x89] = {"ADC", &APU::ADC, AddMode::ZeroPage, 6};
	opTable[0x8A] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x8B] = {"DEC", &APU::DEC, AddMode::ZeroPage, 4};
	opTable[0x8C] = {"DEC", &APU::DEC, AddMode::Absolute, 5};
	opTable[0x8D] = {"LDY", &APU::LDY, AddMode::Immediate, 2};
	opTable[0x8E] = {"PLP", &APU::PLP, AddMode::Implied, 4};
	opTable[0x8F] = {"MOV", &APU::MOV, AddMode::Immediate, 5};
	
	opTable[0x90] = {"BCC", &APU::BCC, AddMode::Implied, 2};
	opTable[0x91] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x92] = {"CLR", &APU::CLR, AddMode::ZeroPage, 4};
	opTable[0x93] = {"BBC", &APU::BBC, AddMode::ZeroPage, 5};
	opTable[0x94] = {"ADC", &APU::ADCA, AddMode::ZeroPageIndexX, 4};
	opTable[0x95] = {"ADC", &APU::ADCA, AddMode::AbsoluteX, 5};
	opTable[0x96] = {"ADC", &APU::ADCA, AddMode::AbsoluteY, 5};
	opTable[0x97] = {"ADC", &APU::ADCA, AddMode::IndirectY, 6};
	opTable[0x98] = {"ADC", &APU::ADC, AddMode::ZeroPage, 5};
	opTable[0x99] = {"ADC", &APU::ADC, AddMode::ZeroPageX, 5};
	opTable[0x9A] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x9B] = {"DEC", &APU::DEC, AddMode::ZeroPageIndexX, 5};
	opTable[0x9C] = {"DEC", &APU::DEC, AddMode::Implied, 2};
	opTable[0x9D] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x9E] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0x9F] = {"XCN", &APU::XCN, AddMode::Implied, 5};
	
	opTable[0xA0] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xA1] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xA2] = {"SET", &APU::SET, AddMode::ZeroPage, 4};
	opTable[0xA3] = {"BBS", &APU::BBS, AddMode::ZeroPage, 5};
	opTable[0xA4] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xA5] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xA6] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xA7] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xA8] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xA9] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xAA] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xAB] = {"INC", &APU::INC, AddMode::ZeroPage, 4};
	opTable[0xAC] = {"INC", &APU::INC, AddMode::Absolute, 5};
	opTable[0xAD] = {"CPY", &APU::CPY, AddMode::Immediate, 2};
	opTable[0xAE] = {"PLA", &APU::PLA, AddMode::Implied, 4};
	opTable[0xAF] = {"STA", &APU::STA, AddMode::ZeroPageIncX, 4};
	
	opTable[0xB0] = {"BCS", &APU::BCS, AddMode::Implied, 2};
	opTable[0xB1] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xB2] = {"CLR", &APU::CLR, AddMode::ZeroPage, 4};
	opTable[0xB3] = {"BBC", &APU::BBC, AddMode::ZeroPage, 5};
	opTable[0xB4] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xB5] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xB6] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xB7] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xB8] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xB9] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xBA] = {"LDW", &APU::LDW, AddMode::Immediate, 5};
	opTable[0xBB] = {"INC", &APU::INC, AddMode::ZeroPageIndexX, 5};
	opTable[0xBC] = {"INC", &APU::INC, AddMode::Implied, 2};
	opTable[0xBD] = {"TXS", &APU::TXS, AddMode::Implied, 2};
	opTable[0xBE] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xBF] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0xC0] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xC1] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xC2] = {"SET", &APU::SET, AddMode::ZeroPage, 4};
	opTable[0xC3] = {"BBS", &APU::BBS, AddMode::ZeroPage, 5};
	opTable[0xC4] = {"STA", &APU::STA, AddMode::ZeroPage, 4};
	opTable[0xC5] = {"STA", &APU::STA, AddMode::Absolute, 5};
	opTable[0xC6] = {"STA", &APU::STA, AddMode::ZeroPageX, 4};
	opTable[0xC7] = {"STA", &APU::STA, AddMode::IndirectX, 2};
	opTable[0xC8] = {"CPX", &APU::CPX, AddMode::Immediate, 2};
	opTable[0xC9] = {"STX", &APU::STX, AddMode::Absolute, 5};
	opTable[0xCA] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xCB] = {"STY", &APU::STY, AddMode::ZeroPage, 4};
	opTable[0xCC] = {"STY", &APU::STY, AddMode::Absolute, 5};
	opTable[0xCD] = {"LDX", &APU::LDX, AddMode::Immediate, 2};
	opTable[0xCE] = {"PLX", &APU::PLX, AddMode::Implied, 4};
	opTable[0xCF] = {"MUL", &APU::MUL, AddMode::Implied, 9};
	
	opTable[0xD0] = {"BNE", &APU::BNE, AddMode::Implied, 2};
	opTable[0xD1] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xD2] = {"CLR", &APU::CLR, AddMode::ZeroPage, 4};
	opTable[0xD3] = {"BBC", &APU::BBC, AddMode::ZeroPage, 5};
	opTable[0xD4] = {"STA", &APU::STA, AddMode::ZeroPageIndexX, 5};
	opTable[0xD5] = {"STA", &APU::STA, AddMode::AbsoluteX, 6};
	opTable[0xD6] = {"STA", &APU::STA, AddMode::AbsoluteY, 6};
	opTable[0xD7] = {"STA", &APU::STA, AddMode::IndirectY, 7};
	opTable[0xD8] = {"STX", &APU::STX, AddMode::ZeroPage, 4};
	opTable[0xD9] = {"STX", &APU::STX, AddMode::ZeroPageIndexY, 5};
	opTable[0xDA] = {"STW", &APU::STW, AddMode::Immediate, 5};
	opTable[0xDB] = {"STY", &APU::STY, AddMode::ZeroPageIndexX, 5};
	opTable[0xDC] = {"DEY", &APU::DEY, AddMode::Implied, 2};
	opTable[0xDD] = {"TYA", &APU::TYA, AddMode::Implied, 2};
	opTable[0xDE] = {"CBNE", &APU::CBNE, AddMode::ZeroPageIndexX, 6};
	opTable[0xDF] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0xE0] = {"CLV", &APU::CLV, AddMode::Implied, 2};
	opTable[0xE1] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xE2] = {"SET", &APU::SET, AddMode::ZeroPage, 4};
	opTable[0xE3] = {"BBS", &APU::BBS, AddMode::ZeroPage, 5};
	opTable[0xE4] = {"LDA", &APU::LDA, AddMode::ZeroPage, 3};
	opTable[0xE5] = {"LDA", &APU::LDA, AddMode::Absolute, 4};
	opTable[0xE6] = {"LDA", &APU::LDA, AddMode::ZeroPageX, 3};
	opTable[0xE7] = {"LDA", &APU::LDA, AddMode::IndirectX, 6};
	opTable[0xE8] = {"LDA", &APU::LDA, AddMode::Immediate, 2};
	opTable[0xE9] = {"LDX", &APU::LDX, AddMode::Absolute, 4};
	opTable[0xEA] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xEB] = {"LDY", &APU::LDY, AddMode::ZeroPage, 3};
	opTable[0xEC] = {"LDY", &APU::LDY, AddMode::Absolute, 4};
	opTable[0xED] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xEE] = {"PLY", &APU::PLY, AddMode::Implied, 4};
	opTable[0xEF] = {"", &APU::XXX, AddMode::Implied, 2};
	
	opTable[0xF0] = {"BEQ", &APU::BEQ, AddMode::Implied, 2};
	opTable[0xF1] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xF2] = {"CLR", &APU::CLR, AddMode::ZeroPage, 4};
	opTable[0xF3] = {"BBC", &APU::BBC, AddMode::ZeroPage, 5};
	opTable[0xF4] = {"LDA", &APU::LDA, AddMode::ZeroPageIndexX, 4};
	opTable[0xF5] = {"LDA", &APU::LDA, AddMode::AbsoluteX, 5};
	opTable[0xF6] = {"LDA", &APU::LDA, AddMode::AbsoluteY, 5};
	opTable[0xF7] = {"LDA", &APU::LDA, AddMode::IndirectY, 6};
	opTable[0xF8] = {"LDX", &APU::LDX, AddMode::ZeroPage, 3};
	opTable[0xF9] = {"LDX", &APU::LDX, AddMode::ZeroPageIndexY, 4};
	opTable[0xFA] = {"", &APU::XXX, AddMode::Implied, 2};
	opTable[0xFB] = {"LDY", &APU::LDY, AddMode::ZeroPageIndexX, 4};
	opTable[0xFC] = {"INY", &APU::INY, AddMode::Implied, 2};
	opTable[0xFD] = {"TAY", &APU::TAY, AddMode::Implied, 2};
	opTable[0xFE] = {"DBNZ", &APU::DBNZ, AddMode::Implied, 4};
	opTable[0xFF] = {"", &APU::XXX, AddMode::Implied, 2};
}

int APU::Clock() {
	int opcycles = 0;

	if (cycles == 0) {
		opcode = ReadAPUMemory(PC);
		
		//Debug();
		
		PC++;

		opcycles = opTable[opcode].cycles;
		opcycles += (this->*opTable[opcode].execute)();
		cycles = opcycles;
	}
	CheckTimers();

	cycles--;
	return opcycles;
}

bool apuEnableLog = false;
FILE* apuLogfile = nullptr;
int apuLogfileMaxSize = 7277;
int apuLogfileLineCount = 0;
void APU::Debug() {
	if (apuLogfile == nullptr && apuEnableLog) apuLogfile = fopen("D:\\C++\\SupaNES\\Test\\APU\\SPC700ADC_log.txt", "w");
	if (apuLogfile) {
		fprintf(apuLogfile, "..%04x %s A:%02x X:%02x Y:%02x SP:%04x YA:%04x %s%s%s%s%s%s%s%s\n",
			PC, opTable[opcode].name, A, X, Y, SP + 0x100, A | (Y << 8),
			(PSW & NFlag) ? "N" : "n",
			(PSW & VFlag) ? "V" : "v",
			(PSW & PFlag) ? "P" : "p",
			(PSW & BFlag) ? "B" : "b",
			(PSW & HFlag) ? "H" : "h",
			(PSW & IFlag) ? "I" : "i",
			(PSW & ZFlag) ? "Z" : "z",
			(PSW & CFlag) ? "C" : "c");
		fflush(apuLogfile);
		//apuLogfileLineCount++;
		if (!apuEnableLog) {
			fclose(apuLogfile);
			assert(false);
		}
	}
	else {
		printf("..%04x %s A:%02x X:%02x Y:%02x SP:%04x YA:%04x %s%s%s%s%s%s%s%s\n",
			PC, opTable[opcode].name, A, X, Y, SP + 0x100, A | (Y << 8),
			(PSW & NFlag) ? "N" : "n",
			(PSW & VFlag) ? "V" : "v",
			(PSW & PFlag) ? "P" : "p",
			(PSW & BFlag) ? "B" : "b",
			(PSW & HFlag) ? "H" : "h",
			(PSW & IFlag) ? "I" : "i",
			(PSW & ZFlag) ? "Z" : "z",
			(PSW & CFlag) ? "C" : "c");
	}
}

void APU::CheckTimers() {
	timer0++;
	timer1++;
	timer2++;

	if (timer0 == 128) {
		timer0 = 0;
		if (registers0[1] & 0x1) {
			timer0Ticks++;
			if (registers1[2] == timer0Ticks) {
				registers1[5]++;
				registers1[5] &= 0xF;
				timer0Ticks = 0;
			}
		}
	}
	if (timer1 == 128) {
		timer1 = 0;
		if (registers0[1] & 0x2) {
			timer1Ticks++;
			if (registers1[3] == timer1Ticks) {
				registers1[6]++;
				registers1[6] &= 0xF;
				timer1Ticks = 0;
			}
		}
	}
	if (timer2 == 16) {
		timer2 = 0;
		if (registers0[1] & 0x4) {
			timer2Ticks++;
			if (registers1[4] == timer2Ticks) {
				registers1[7]++;
				registers1[7] &= 0xF;
				timer2Ticks = 0;
			}
		}
	}
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
			if (address >= 0xFD) {
				registers1[address - 0xF8] = 0;
			}
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
			if (address == 0xF1) {
				// Writting always reset
				timer0Ticks = 0;
				timer1Ticks = 0;
				timer2Ticks = 0;

				// Reset ports 2 & 3
				if (value & 0x20) {
					CPUPorts[2] = 0;
					CPUPorts[3] = 0;
				}
				// Reset ports 0 & 1
				if (value & 0x10) {
					CPUPorts[0] = 0;
					CPUPorts[1] = 0;
				}
			}
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

	//printf("Reading APU Port %02x: %02x\n", address, APUPorts[index]);

	return APUPorts[index];
}

void APU::WriteCPUPorts(uint8_t address, uint8_t value) {
	uint8_t index = address & 0x3;

	CPUPorts[index] = value;
	
	//printf("Writting APU Port %02x: %02x\n", address, value);
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

uint8_t APU::PullStack() {
	return ReadAPUMemory(++SP + 0x100);
}

void APU::PushStack(uint8_t value) {
	WriteAPUMemory(SP-- + 0x100, value);
}

void APU::FetchValue(AddMode addMode) {
	switch (addMode) {
		case AddMode::Implied:
			fetched = A;
			break;

		case AddMode::Immediate:
			IMM();
			break;

		case AddMode::ZeroPage:
			ZeroPage();

			fetched = ReadAPUMemory(fetchAddress);
			break;

		case AddMode::ZeroPageIndexX:
			ZeroPageIndexX();

			fetched = ReadAPUMemory(fetchAddress);
			break;

		case AddMode::ZeroPageIndexY:
			ZeroPageIndexY();

			fetched = ReadAPUMemory(fetchAddress);
			break;

		case AddMode::ZeroPageX:
			ZeroPageX();

			fetched = ReadAPUMemory(fetchAddress);
			break;

		case AddMode::ZeroPageY:
			ZeroPageY();

			fetched = ReadAPUMemory(fetchAddress);
			break;

		case AddMode::Absolute:
			Absolute();

			fetched = ReadAPUMemory(fetchAddress);
			break;

		case AddMode::AbsoluteX:
			AbsoluteX();

			fetched = ReadAPUMemory(fetchAddress);
			break;

		case AddMode::AbsoluteY:
			AbsoluteY();

			fetched = ReadAPUMemory(fetchAddress);
			break;

		case AddMode::IndirectX:
			IndirectX();

			fetched = ReadAPUMemory(fetchAddress);
			break;

		case AddMode::IndirectY:
			IndirectY();

			fetched = ReadAPUMemory(fetchAddress);
			break;

		default:
			printf("%d\n", addMode);
			assert(false);
	}
}

void APU::WriteValue(AddMode addMode, uint8_t value, bool read) {
	switch (addMode) {
		case AddMode::Implied:
			A = value;
			break;

		case AddMode::ZeroPage:
			if (!read) {
				ZeroPage();
			}

			WriteAPUMemory(fetchAddress, value);
			break;

		case AddMode::ZeroPageIndexX:
			if (!read) {
				ZeroPageIndexX();
			}

			WriteAPUMemory(fetchAddress, value);
			break;

		case AddMode::ZeroPageIndexY:
			if (!read) {
				ZeroPageIndexY();
			}

			WriteAPUMemory(fetchAddress, value);
			break;

		case AddMode::ZeroPageX:
			ZeroPageX();

			WriteAPUMemory(fetchAddress, value);
			break;

		case AddMode::ZeroPageIncX:
			ZeroPageIncX();

			WriteAPUMemory(fetchAddress, value);
			break;

		case AddMode::Absolute:
			if (!read) {
				Absolute();
			}

			WriteAPUMemory(fetchAddress, value);
			break;

		case AddMode::AbsoluteX:
			AbsoluteX();

			WriteAPUMemory(fetchAddress, value);
			break;

		case AddMode::AbsoluteY:
			AbsoluteY();

			WriteAPUMemory(fetchAddress, value);
			break;

		case AddMode::IndirectX:
		{
			IndirectX();

			WriteAPUMemory(fetchAddress, value);
			break;
		}

		case AddMode::IndirectY:
		{
			IndirectY();

			WriteAPUMemory(fetchAddress, value);
			break;
		}

		default:
			printf("%d\n", addMode);
			assert(false);
	}
}

void APU::IMM() {
	fetched = ReadAPUMemory(PC++);
}

void APU::ZeroPage() {
	fetchAddress = ReadAPUMemory(PC++);
	if (PSW & PFlag) {
		fetchAddress += 0x100;
	}
}

void APU::ZeroPageX() {
	fetchAddress = X;
	if (PSW & PFlag) {
		fetchAddress += 0x100;
	}
}

void APU::ZeroPageIncX() {
	fetchAddress = X;
	if (PSW & PFlag) {
		fetchAddress += 0x100;
	}
	X++;
}

void APU::ZeroPageIndexX() {
	fetchAddress = ReadAPUMemory(PC++);
	fetchAddress += X;
	fetchAddress &= 0xFF;

	if (PSW & PFlag) {
		fetchAddress += 0x100;
	}
}

void APU::ZeroPageIndexY() {
	fetchAddress = ReadAPUMemory(PC++);
	fetchAddress += Y;
	fetchAddress &= 0xFF;

	if (PSW & PFlag) {
		fetchAddress += 0x100;
	}
}

void APU::ZeroPageY() {
	fetchAddress = Y;
	if (PSW & PFlag) {
		fetchAddress += 0x100;
	}
}

void APU::Absolute() {
	fetchAddress = ReadAPUMemory(PC++);
	fetchAddress |= ReadAPUMemory(PC++) << 8;
}

void APU::AbsoluteX() {
	fetchAddress = ReadAPUMemory(PC++);
	fetchAddress |= ReadAPUMemory(PC++) << 8;
	fetchAddress += X;
}

void APU::AbsoluteY() {
	fetchAddress = ReadAPUMemory(PC++);
	fetchAddress |= ReadAPUMemory(PC++) << 8;
	fetchAddress += Y;
}

void APU::IndirectX() {
	uint16_t indAddress = ReadAPUMemory(PC++);
	indAddress += X;
	indAddress &= 0xFF;
	if (PSW & PFlag) {
		indAddress += 0x100;
	}
	fetchAddress = ReadAPUMemory(indAddress);
	fetchAddress |= ReadAPUMemory(indAddress + 1) << 8;
}

void APU::IndirectY() {
	uint16_t indAddress = ReadAPUMemory(PC++);
	if (PSW & PFlag) {
		indAddress += 0x100;
	}
	fetchAddress = ReadAPUMemory(indAddress);
	fetchAddress |= ReadAPUMemory(indAddress + 1) << 8;
	fetchAddress += Y;
}

// Opcodes

uint8_t APU::ADCA() {
	FetchValue(opTable[opcode].addMode);

	uint8_t carry = (PSW & CFlag) ? 1 : 0;

	uint8_t result = A + fetched + carry;

	CheckNFlag(result);
	CheckZFlag(result);

	SetFlag(CFlag, result < A);
	SetFlag(HFlag, ((A & 0xF) + (fetched & 0xF) + carry) & 0x10);

	SetFlag(VFlag, (int8_t)result < -128 || result > 127);

	A = result;

	return 0;
}

uint8_t APU::ADCW() {
	uint8_t address = ReadAPUMemory(PC++);
	uint16_t offset = 0;
	if (PSW & PFlag) {
		offset += 0x100;
	}

	uint16_t value = ReadAPUMemory(address + offset);
	value |= ReadAPUMemory((address + 1) + offset) << 8;

	uint16_t YA = A + (Y << 8);
	uint16_t result = YA + value;

	CheckNFlag(result >> 8);
	SetFlag(ZFlag, result == 0);

	SetFlag(CFlag, result < YA);
	SetFlag(HFlag, ((YA & 0xFFF) + (value & 0xFFF)) & 0x1000);

	SetFlag(VFlag, (int16_t)result < -32768 || result > 32767);

	A = result & 0xFF;
	Y = result >> 8;

	return 0;
}

uint8_t APU::ADC() {
	uint8_t value = 0;
	if (opcode == 0x99) {
		FetchValue(AddMode::ZeroPageY);
		value = fetched;
	}
	else if (opcode == 0x98) {
		FetchValue(AddMode::Immediate);
		value = fetched;
	}
	else if (opcode == 0x89) {
		FetchValue(AddMode::ZeroPage);
		value = fetched;
	}
	FetchValue(opTable[opcode].addMode);

	uint8_t carry = (PSW & CFlag) ? 1 : 0;

	uint8_t result = fetched + value + carry;

	CheckNFlag(result);
	CheckZFlag(result);

	SetFlag(CFlag, result < A);
	SetFlag(HFlag, ((A & 0xF) + (fetched & 0xF) + 1) & 0x10);

	SetFlag(VFlag, (int8_t)result < -128 || result > 127);

	WriteValue(opTable[opcode].addMode, result, true);

	return 0;
}

uint8_t APU::ANDA() {
	FetchValue(opTable[opcode].addMode);
	A &= fetched;

	CheckNFlag(A);
	CheckZFlag(A);

	return 0;
}

uint8_t APU::AND() {
	uint8_t value = 0;
	if (opcode == 0x39) {
		FetchValue(AddMode::ZeroPageY);
		value = fetched;
	}
	else if (opcode == 0x38) {
		FetchValue(AddMode::Immediate);
		value = fetched;
	}
	else if (opcode == 0x29) {
		FetchValue(AddMode::ZeroPage);
		value = fetched;
	}
	FetchValue(opTable[opcode].addMode);

	fetched &= value;

	CheckNFlag(fetched);
	CheckZFlag(fetched);

	WriteValue(opTable[opcode].addMode, fetched, true);

	return 0;
}

uint8_t APU::ANDC() {
	uint16_t address = ReadAPUMemory(PC++);
	address |= ReadAPUMemory(PC++) << 8;

	uint8_t bit = (address & 0xE000) >> 13;
	address &= 0x1FFF;

	uint8_t value = ReadAPUMemory(address);

	if (PSW & CFlag) {
		if (opcode == 0x4A) {
			SetFlag(CFlag, value & (1 << bit));
		}
		else {
			SetFlag(CFlag, (value & (1 << bit)) == 0);
		}
	}

	return 0;
}

uint8_t APU::ASL() {
	FetchValue(opTable[opcode].addMode);

	SetFlag(CFlag, fetched & 0x80);

	fetched <<= 1;

	CheckNFlag(fetched);
	CheckZFlag(fetched);

	WriteValue(opTable[opcode].addMode, fetched, true);

	return 0;
}

uint8_t APU::BBC() {
	FetchValue(opTable[opcode].addMode);

	int8_t offset = ReadAPUMemory(PC++);

	uint8_t bit = (opcode - 0x13) >> 5;
	bit = 1 << bit;

	if (!(fetched & bit)) {
		PC += offset;
		return 2;
	}
	return 0;
}

uint8_t APU::BBS() {
	FetchValue(opTable[opcode].addMode);

	int8_t offset = ReadAPUMemory(PC++);

	uint8_t bit = (opcode - 0x03) >> 5;
	bit = 1 << bit;

	if (fetched & bit) {
		PC += offset;
		return 2;
	}
	return 0;
}

uint8_t APU::BCC() {
	int8_t offset = ReadAPUMemory(PC++);
	if (!(PSW & CFlag)) {
		PC += offset;
		return 2;
	}

	return 0;
}

uint8_t APU::BCS() {
	int8_t offset = ReadAPUMemory(PC++);
	if (PSW & CFlag) {
		PC += offset;
		return 2;
	}

	return 0;
}

uint8_t APU::BEQ() {
	int8_t offset = ReadAPUMemory(PC++);
	if (PSW & ZFlag) {
		PC += offset;
		return 2;
	}

	return 0;
}

uint8_t APU::BMI() {
	int8_t offset = ReadAPUMemory(PC++);
	if (PSW & NFlag) {
		PC += offset;
		return 2;
	}

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

uint8_t APU::CBNE() {
	FetchValue(opTable[opcode].addMode);
	int8_t offset = ReadAPUMemory(PC++);

	if (A != fetched) {
		PC += offset;
		return 2;
	}

	return 0;
}

uint8_t APU::CLC() {
	SetFlag(CFlag, false);

	return 0;
}

uint8_t APU::CLP() {
	SetFlag(PFlag, false);

	return 0;
}

uint8_t APU::CLR() {
	FetchValue(opTable[opcode].addMode);
	uint8_t bit = (opcode - 0x12) >> 5;

	fetched &= ~(1 << bit);

	WriteValue(opTable[opcode].addMode, fetched, true);

	return 0;
}

uint8_t APU::TCLR() {
	FetchValue(opTable[opcode].addMode);

	uint8_t result = A - fetched;
	CheckNFlag(result);
	CheckZFlag(result);

	WriteValue(opTable[opcode].addMode, fetched & ~A, true);

	return 0;
}

uint8_t APU::CLV() {
	PSW &= ~(VFlag | HFlag);

	return 0;
}

uint8_t APU::CMP() {
	FetchValue(opTable[opcode].addMode);
	
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

	return 0;
}

uint8_t APU::CPA() {
	FetchValue(opTable[opcode].addMode);
	
	uint8_t result = A - fetched;

	CheckNFlag(result);
	CheckZFlag(result);

	if (result > A) {
		PSW &= ~CFlag;
	} else {
		PSW |= CFlag;
	}

	return 0;
}

uint8_t APU::CPW() {
	uint8_t address = ReadAPUMemory(PC++);
	uint16_t offset = 0;
	if (PSW & PFlag) {
		offset += 0x100;
	}

	uint16_t value = ReadAPUMemory(address + offset);
	value |= ReadAPUMemory((address + 1) + offset) << 8;

	uint16_t YA = A + (Y << 8);
	uint16_t result = YA - value;

	CheckNFlag(result >> 8);
	SetFlag(ZFlag, result == 0);

	if (result > YA) {
		PSW &= ~CFlag;
	}
	else {
		PSW |= CFlag;
	}

	return 0;
}

uint8_t APU::CPX() {
	FetchValue(opTable[opcode].addMode);
	
	uint8_t result = X - fetched;

	CheckNFlag(result);
	CheckZFlag(result);

	if (result > X) {
		PSW &= ~CFlag;
	} else {
		PSW |= CFlag;
	}

	return 0;
}

uint8_t APU::CPY() {
	FetchValue(opTable[opcode].addMode);
	
	uint8_t result = Y - fetched;

	CheckNFlag(result);
	CheckZFlag(result);

	if (result > Y) {
		PSW &= ~CFlag;
	} else {
		PSW |= CFlag;
	}

	return 0;
}

uint8_t APU::DBNZ() {
	if (opcode == 0x6E) {
		FetchValue(opTable[opcode].addMode);
	} else {
		Y--;
	}
	int8_t offset = ReadAPUMemory(PC++);

	if (opcode == 0x6E) {
		fetched--;
		WriteValue(opTable[opcode].addMode, fetched, true);
		if (fetched != 0) {
			PC += offset;
			return 2;
		}
	} else {
		if (Y != 0) {
			PC += offset;
			return 2;
		}
	}
	return 0;
}

uint8_t APU::DEC() {
	FetchValue(opTable[opcode].addMode);

	fetched--;

	CheckNFlag(fetched);
	CheckZFlag(fetched);

	WriteValue(opTable[opcode].addMode, fetched, true);

	return 0;
}

uint8_t APU::DEW() {
	uint8_t address = ReadAPUMemory(PC++);
	uint16_t offset = 0;
	if (PSW & PFlag) {
		offset += 0x100;
	}

	uint16_t value = ReadAPUMemory(address + offset);
	value |= ReadAPUMemory((address + 1) + offset) << 8;

	value--;

	SetFlag(NFlag, value & 0x8000);
	SetFlag(ZFlag, value == 0);

	WriteAPUMemory(address + offset, value & 0xFF);
	WriteAPUMemory((address + 1) + offset, value >> 8);

	return 0;
}

uint8_t APU::DEX() {
	X--;

	CheckNFlag(X);
	CheckZFlag(X);

	return 0;
}

uint8_t APU::DEY() {
	Y--;

	CheckNFlag(Y);
	CheckZFlag(Y);

	return 0;
}

uint8_t APU::EORA() {
	FetchValue(opTable[opcode].addMode);
	A ^= fetched;

	CheckNFlag(A);
	CheckZFlag(A);

	return 0;
}

uint8_t APU::INC() {
	FetchValue(opTable[opcode].addMode);

	fetched++;

	CheckNFlag(fetched);
	CheckZFlag(fetched);

	WriteValue(opTable[opcode].addMode, fetched, true);

	return 0;
}

uint8_t APU::INW() {
	uint8_t address = ReadAPUMemory(PC++);
	uint16_t offset = 0;
	if (PSW & PFlag) {
		offset += 0x100;
	}

	uint16_t value = ReadAPUMemory(address + offset);
	value |= ReadAPUMemory((address + 1) + offset) << 8;

	value++;

	SetFlag(NFlag, value & 0x8000);
	SetFlag(ZFlag, value == 0);

	WriteAPUMemory(address + offset, value & 0xFF);
	WriteAPUMemory((address + 1) + offset, value >> 8);

	return 0;
}

uint8_t APU::INX() {
	X++;

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

uint8_t APU::JMP() {
	uint16_t jumpAddress = ReadAPUMemory(PC++);
	jumpAddress |= ReadAPUMemory(PC++) << 8;
	if (opcode == 0x1F) {
		jumpAddress += X;
		jumpAddress = ReadAPUMemory(jumpAddress) | (ReadAPUMemory(jumpAddress + 1) << 8);
	}

	PC = jumpAddress;

	return 0;
}

uint8_t APU::JSR() {
	uint16_t jumpAddress = ReadAPUMemory(PC++);
	jumpAddress |= ReadAPUMemory(PC++) << 8;

	PushStack(PC >> 8);
	PushStack(PC);

	PC = jumpAddress;

	return 0;
}

uint8_t APU::LDA() {
	FetchValue(opTable[opcode].addMode);

	A = fetched;

	CheckNFlag(A);
	CheckZFlag(A);

	return 0;
}

uint8_t APU::LDX() {
	FetchValue(opTable[opcode].addMode);

	X = fetched;

	CheckNFlag(X);
	CheckZFlag(X);

	return 0;
}

uint8_t APU::LDY() {
	FetchValue(opTable[opcode].addMode);

	Y = fetched;

	CheckNFlag(Y);
	CheckZFlag(Y);

	return 0;
}

uint8_t APU::LDW() {
	FetchValue(opTable[opcode].addMode);

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

uint8_t APU::LSR() {
	FetchValue(opTable[opcode].addMode);
	SetFlag(CFlag, fetched & 0x01);
	fetched >>= 1;

	WriteValue(opTable[opcode].addMode, fetched, true);

	return 0;
}

uint8_t APU::MOV() {
	FetchValue(opTable[opcode].addMode);
	
	uint16_t address = ReadAPUMemory(PC++);

	if (PSW & PFlag) {
		address += 0x100;
	}

	WriteAPUMemory(address, fetched);

	return 0;
}

uint8_t APU::MUL() {
	uint16_t result = Y * A;
	A = result & 0xFF;
	Y = result >> 8;

	CheckNFlag(Y);
	CheckZFlag(Y);

	return 0;
}

uint8_t APU::ORA() {
	FetchValue(opTable[opcode].addMode);
	A |= fetched;

	CheckNFlag(A);
	CheckZFlag(A);

	return 0;
}

uint8_t APU::PHA() {
	PushStack(A);

	return 0;
}

uint8_t APU::PHP() {
	PushStack(PSW);

	return 0;
}

uint8_t APU::PHX() {
	PushStack(X);

	return 0;
}

uint8_t APU::PHY() {
	PushStack(Y);

	return 0;
}

uint8_t APU::PLA() {
	A = PullStack();

	return 0;
}

uint8_t APU::PLP() {
	PSW = PullStack();

	return 0;
}

uint8_t APU::PLX() {
	X = PullStack();

	return 0;
}

uint8_t APU::PLY() {
	Y = PullStack();

	return 0;
}

uint8_t APU::RTS() {
	PC = PullStack();
	PC |= PullStack() << 8;

	return 0;
}

uint8_t APU::SEC() {
	SetFlag(CFlag, true);

	return 0;
}

uint8_t APU::SEP() {
	SetFlag(PFlag, true);

	return 0;
}

uint8_t APU::SET() {
	FetchValue(opTable[opcode].addMode);
	uint8_t bit = (opcode - 0x2) >> 5;
	fetched |= 1 << bit;

	WriteValue(opTable[opcode].addMode, fetched, true);

	return 0;
}

uint8_t APU::TSET() {
	FetchValue(opTable[opcode].addMode);

	uint8_t result = A - fetched;
	CheckNFlag(result);
	CheckZFlag(result);

	WriteValue(opTable[opcode].addMode, fetched | A, true);

	return 0;
}

uint8_t APU::STA() {
	WriteValue(opTable[opcode].addMode, A);

	return 0;
}

uint8_t APU::STW() {
	FetchValue(opTable[opcode].addMode);

	uint16_t address = 0;
	if (PSW & PFlag) {
		address += 0x100;
	}

	WriteAPUMemory(address + fetched, A);
	WriteAPUMemory(address + (fetched + 1), Y);

	return 0;
}

uint8_t APU::STX() {
	WriteValue(opTable[opcode].addMode, X);

	return 0;
}

uint8_t APU::STY() {
	WriteValue(opTable[opcode].addMode, Y);

	return 0;
}

uint8_t APU::TAX() {
	X = A;

	CheckNFlag(X);
	CheckZFlag(X);

	return 0;
}

uint8_t APU::TAY() {
	Y = A;

	CheckNFlag(Y);
	CheckZFlag(Y);

	return 0;
}

uint8_t APU::TXA() {
	A = X;

	CheckNFlag(A);
	CheckZFlag(A);

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

uint8_t APU::XCN() {
	A = (A >> 4) | (A << 4);

	CheckNFlag(A);
	CheckZFlag(A);

	return 0;
}

uint8_t APU::XXX() {
	printf("APU OP: %02x\n", opcode);
	assert(false);
	return 0;
}
