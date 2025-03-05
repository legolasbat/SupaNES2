#include "CPU.h"
#include "SupaNES.h"

#include <cassert>
#include <cstdint>
#include <stdio.h>

#define NFlag 0x80	// Negative flag
#define VFlag 0x40	// Overflow flag
#define MFlag 0x20	// Accumulator size flag (0=16bit, 1=8bit)
#define XFlag 0x10	// Index size flag. Break flag in emulation mode only
#define DFlag 0x08	// Unused. Decimal flag
#define IFlag 0x04	// IRQ Disable flag
#define ZFlag 0x02	// Zero flag
#define CFlag 0x01	// Carry flag

CPU::CPU(SupaNES* supaNes) {
	memory = supaNes;

	emulationMode = true;

	regs.PC = 0x8000;

	regs.A.HL = 0;
	regs.X.HL = 0;
	regs.Y.HL = 0;

	regs.SP = 0x01FF;

	regs.DP = 0;

	regs.PB = 0;

	regs.DB = 0;

	regs.P = 0x34;

	joypad1Low = 0;
	joypad1High = 0;

	opTable[0x00] = {"BRK", &CPU::BRK, AddMode::Implied, 7};
	opTable[0x01] = {"ORA", &CPU::ORA, AddMode::DirectPageIndirectX, 6};
	opTable[0x02] = {"COP", &CPU::COP, AddMode::Implied, 7};
	opTable[0x03] = {"ORA", &CPU::ORA, AddMode::StackRelative, 4};
	opTable[0x04] = {"TSB", &CPU::TSB, AddMode::DirectPage, 5};
	opTable[0x05] = {"ORA", &CPU::ORA, AddMode::DirectPage, 3};
	opTable[0x06] = {"ASL", &CPU::ASL, AddMode::DirectPage, 5};
	opTable[0x07] = {"ORA", &CPU::ORA, AddMode::DirectPageIndirectLong, 6};
	opTable[0x08] = {"PHP", &CPU::PHP, AddMode::Implied, 3};
	opTable[0x09] = {"ORA", &CPU::ORA, AddMode::Immediate, 2};
	opTable[0x0A] = {"ASL", &CPU::ASL, AddMode::Implied, 2};
	opTable[0x0B] = {"PHD", &CPU::PHD, AddMode::Implied, 4};
	opTable[0x0C] = {"TSB", &CPU::TSB, AddMode::Absolute, 6};
	opTable[0x0D] = {"ORA", &CPU::ORA, AddMode::Absolute, 4};
	opTable[0x0E] = {"ASL", &CPU::ASL, AddMode::Absolute, 6};
	opTable[0x0F] = {"ORA", &CPU::ORA, AddMode::AbsoluteLong, 5};

	opTable[0x10] = {"BPL", &CPU::BPL, AddMode::Implied, 2};
	opTable[0x11] = {"ORA", &CPU::ORA, AddMode::DirectPageIndirectIndexedY, 5};
	opTable[0x12] = {"ORA", &CPU::ORA, AddMode::DirectPageIndirect, 5};
	opTable[0x13] = {"ORA", &CPU::ORA, AddMode::StackRelativeIndirectIndexedY, 7};
	opTable[0x14] = {"TRB", &CPU::TRB, AddMode::DirectPage, 5};
	opTable[0x15] = {"ORA", &CPU::ORA, AddMode::DirectPageIndexedX, 4};
	opTable[0x16] = {"ASL", &CPU::ASL, AddMode::DirectPageIndexedX, 6};
	opTable[0x17] = {"ORA", &CPU::ORA, AddMode::DirectPageIndirectLongIndexedY, 6};
	opTable[0x18] = {"CLC", &CPU::CLC, AddMode::Implied, 2};
	opTable[0x19] = {"ORA", &CPU::ORA, AddMode::AbsoluteIndexedY, 4};
	opTable[0x1A] = {"INC", &CPU::INC, AddMode::Implied, 2};
	opTable[0x1B] = {"TCS", &CPU::TCS, AddMode::Implied, 2};
	opTable[0x1C] = {"TRB", &CPU::TRB, AddMode::Absolute, 6};
	opTable[0x1D] = {"ORA", &CPU::ORA, AddMode::AbsoluteIndexedX, 4};
	opTable[0x1E] = {"ASL", &CPU::ASL, AddMode::AbsoluteIndexedX, 7};
	opTable[0x1F] = {"ORA", &CPU::ORA, AddMode::AbsoluteLongIndexedX, 5};

	opTable[0x20] = {"JSR", &CPU::JSR, AddMode::Absolute, 6};
	opTable[0x21] = {"AND", &CPU::AND, AddMode::DirectPageIndirectX, 6};
	opTable[0x22] = {"JSL", &CPU::JSL, AddMode::AbsoluteLong, 8};
	opTable[0x23] = {"AND", &CPU::AND, AddMode::StackRelative, 4};
	opTable[0x24] = {"BIT", &CPU::BIT, AddMode::DirectPage, 3};
	opTable[0x25] = {"AND", &CPU::AND, AddMode::DirectPage, 3};
	opTable[0x26] = {"ROL", &CPU::ROL, AddMode::DirectPage, 5};
	opTable[0x27] = {"AND", &CPU::AND, AddMode::DirectPageIndirectLong, 6};
	opTable[0x28] = {"PLP", &CPU::PLP, AddMode::Implied, 4};
	opTable[0x29] = {"AND", &CPU::AND, AddMode::Immediate, 2};
	opTable[0x2A] = {"ROL", &CPU::ROL, AddMode::Implied, 2};
	opTable[0x2B] = {"PLD", &CPU::PLD, AddMode::Implied, 5};
	opTable[0x2C] = {"BIT", &CPU::BIT, AddMode::Absolute, 4};
	opTable[0x2D] = {"AND", &CPU::AND, AddMode::Absolute, 4};
	opTable[0x2E] = {"ROL", &CPU::ROL, AddMode::Absolute, 6};
	opTable[0x2F] = {"AND", &CPU::AND, AddMode::AbsoluteLong, 5};

	opTable[0x30] = {"BMI", &CPU::BMI, AddMode::Implied, 2};
	opTable[0x31] = {"AND", &CPU::AND, AddMode::DirectPageIndirectIndexedY, 5};
	opTable[0x32] = {"AND", &CPU::AND, AddMode::DirectPageIndirect, 5};
	opTable[0x33] = {"AND", &CPU::AND, AddMode::StackRelativeIndirectIndexedY, 7};
	opTable[0x34] = {"BIT", &CPU::BIT, AddMode::DirectPageIndexedX, 4};
	opTable[0x35] = {"AND", &CPU::AND, AddMode::DirectPageIndexedX, 4};
	opTable[0x36] = {"ROL", &CPU::ROL, AddMode::DirectPageIndexedX, 6};
	opTable[0x37] = {"AND", &CPU::AND, AddMode::DirectPageIndirectLongIndexedY, 6};
	opTable[0x38] = {"SEC", &CPU::SEC, AddMode::Implied, 2};
	opTable[0x39] = {"AND", &CPU::AND, AddMode::AbsoluteIndexedY, 4};
	opTable[0x3A] = {"DEC", &CPU::DEC, AddMode::Implied, 2};
	opTable[0x3B] = {"TSC", &CPU::TSC, AddMode::Implied, 2};
	opTable[0x3C] = {"BIT", &CPU::BIT, AddMode::AbsoluteIndexedX, 4};
	opTable[0x3D] = {"AND", &CPU::AND, AddMode::AbsoluteIndexedX, 4};
	opTable[0x3E] = {"ROL", &CPU::ROL, AddMode::AbsoluteIndexedX, 7};
	opTable[0x3F] = {"AND", &CPU::AND, AddMode::AbsoluteLongIndexedX, 5};

	opTable[0x40] = {"RTI", &CPU::RTI, AddMode::Implied, 6};
	opTable[0x41] = {"EOR", &CPU::EOR, AddMode::DirectPageIndirectX, 6};
	opTable[0x42] = {"WDM", &CPU::WDM, AddMode::Immediate, 2};	// Unused
	opTable[0x43] = {"EOR", &CPU::EOR, AddMode::StackRelative, 4};
	opTable[0x44] = {"MVP", &CPU::MVP, AddMode::Implied, 0};
	opTable[0x45] = {"EOR", &CPU::EOR, AddMode::DirectPage, 3};
	opTable[0x46] = {"LSR", &CPU::LSR, AddMode::DirectPage, 5};
	opTable[0x47] = {"EOR", &CPU::EOR, AddMode::DirectPageIndirectLong, 6};
	opTable[0x48] = {"PHA", &CPU::PHA, AddMode::Implied, 3};
	opTable[0x49] = {"EOR", &CPU::EOR, AddMode::Immediate, 2};
	opTable[0x4A] = {"LSR", &CPU::LSR, AddMode::Implied, 2};
	opTable[0x4B] = {"PHK", &CPU::PHK, AddMode::Implied, 3};
	opTable[0x4C] = {"JMP", &CPU::JMP, AddMode::Absolute, 3};
	opTable[0x4D] = {"EOR", &CPU::EOR, AddMode::Absolute, 4};
	opTable[0x4E] = {"LSR", &CPU::LSR, AddMode::Absolute, 6};
	opTable[0x4F] = {"EOR", &CPU::EOR, AddMode::AbsoluteLong, 5};

	opTable[0x50] = {"BVC", &CPU::BVC, AddMode::Implied, 2};
	opTable[0x51] = {"EOR", &CPU::EOR, AddMode::DirectPageIndirectIndexedY, 5};
	opTable[0x52] = {"EOR", &CPU::EOR, AddMode::DirectPageIndirect, 5};
	opTable[0x53] = {"EOR", &CPU::EOR, AddMode::StackRelativeIndirectIndexedY, 7};
	opTable[0x54] = {"MVN", &CPU::MVN, AddMode::Implied, 0};
	opTable[0x55] = {"EOR", &CPU::EOR, AddMode::DirectPageIndexedX, 4};
	opTable[0x56] = {"LSR", &CPU::LSR, AddMode::DirectPageIndexedX, 6};
	opTable[0x57] = {"EOR", &CPU::EOR, AddMode::DirectPageIndirectLongIndexedY, 6};
	opTable[0x58] = {"CLI", &CPU::CLI, AddMode::Implied, 2};
	opTable[0x59] = {"EOR", &CPU::EOR, AddMode::AbsoluteIndexedY, 4};
	opTable[0x5A] = {"PHY", &CPU::PHY, AddMode::Implied, 3};
	opTable[0x5B] = {"TCD", &CPU::TCD, AddMode::Implied, 2};
	opTable[0x5C] = {"JML", &CPU::JML, AddMode::AbsoluteLong, 4};
	opTable[0x5D] = {"EOR", &CPU::EOR, AddMode::AbsoluteIndexedX, 4};
	opTable[0x5E] = {"LSR", &CPU::LSR, AddMode::AbsoluteIndexedX, 7};
	opTable[0x5F] = {"EOR", &CPU::EOR, AddMode::AbsoluteLongIndexedX, 5};

	opTable[0x60] = {"RTS", &CPU::RTS, AddMode::Implied, 6};
	opTable[0x61] = {"ADC", &CPU::ADC, AddMode::DirectPageIndirectX, 6};
	opTable[0x62] = {"PER", &CPU::PER, AddMode::Implied, 6};
	opTable[0x63] = {"ADC", &CPU::ADC, AddMode::StackRelative, 4};
	opTable[0x64] = {"STZ", &CPU::STZ, AddMode::DirectPage, 3};
	opTable[0x65] = {"ADC", &CPU::ADC, AddMode::DirectPage, 3};
	opTable[0x66] = {"ROR", &CPU::ROR, AddMode::DirectPage, 5};
	opTable[0x67] = {"ADC", &CPU::ADC, AddMode::DirectPageIndirectLong, 6};
	opTable[0x68] = {"PLA", &CPU::PLA, AddMode::Implied, 4};
	opTable[0x69] = {"ADC", &CPU::ADC, AddMode::Immediate, 2};
	opTable[0x6A] = {"ROR", &CPU::ROR, AddMode::Implied, 2};
	opTable[0x6B] = {"RTL", &CPU::RTL, AddMode::Implied, 6};
	opTable[0x6C] = {"JMP", &CPU::JMP, AddMode::AbsoluteIndirect, 5};
	opTable[0x6D] = {"ADC", &CPU::ADC, AddMode::Absolute, 4};
	opTable[0x6E] = {"ROR", &CPU::ROR, AddMode::Absolute, 6};
	opTable[0x6F] = {"ADC", &CPU::ADC, AddMode::AbsoluteLong, 5};

	opTable[0x70] = {"BVS", &CPU::BVS, AddMode::Implied, 2};
	opTable[0x71] = {"ADC", &CPU::ADC, AddMode::DirectPageIndirectIndexedY, 5};
	opTable[0x72] = {"ADC", &CPU::ADC, AddMode::DirectPageIndirect, 5};
	opTable[0x73] = {"ADC", &CPU::ADC, AddMode::StackRelativeIndirectIndexedY, 7};
	opTable[0x74] = {"STZ", &CPU::STZ, AddMode::DirectPageIndexedX, 4};
	opTable[0x75] = {"ADC", &CPU::ADC, AddMode::DirectPageIndexedX, 4};
	opTable[0x76] = {"ROR", &CPU::ROR, AddMode::DirectPageIndexedX, 6};
	opTable[0x77] = {"ADC", &CPU::ADC, AddMode::DirectPageIndirectLongIndexedY, 6};
	opTable[0x78] = {"SEI", &CPU::SEI, AddMode::Implied, 2};
	opTable[0x79] = {"ADC", &CPU::ADC, AddMode::AbsoluteIndexedY, 4};
	opTable[0x7A] = {"PLY", &CPU::PLY, AddMode::Implied, 4};
	opTable[0x7B] = {"TDC", &CPU::TDC, AddMode::Implied, 2};
	opTable[0x7C] = {"JMP", &CPU::JMP, AddMode::AbsoluteIndexedIndirectX, 6};
	opTable[0x7D] = {"ADC", &CPU::ADC, AddMode::AbsoluteIndexedX, 4};
	opTable[0x7E] = {"ROR", &CPU::ROR, AddMode::AbsoluteIndexedX, 7};
	opTable[0x7F] = {"ADC", &CPU::ADC, AddMode::AbsoluteLongIndexedX, 5};

	opTable[0x80] = {"BRA", &CPU::BRA, AddMode::Implied, 3};
	opTable[0x81] = {"STA", &CPU::STA, AddMode::DirectPageIndirectX, 6};
	opTable[0x82] = {"BRL", &CPU::BRL, AddMode::Implied, 4};
	opTable[0x83] = {"STA", &CPU::STA, AddMode::StackRelative, 4};
	opTable[0x84] = {"STY", &CPU::STY, AddMode::DirectPage, 3};
	opTable[0x85] = {"STA", &CPU::STA, AddMode::DirectPage, 3};
	opTable[0x86] = {"STX", &CPU::STX, AddMode::DirectPage, 3};
	opTable[0x87] = {"STA", &CPU::STA, AddMode::DirectPageIndirectLong, 6};
	opTable[0x88] = {"DEY", &CPU::DEY, AddMode::Implied, 2};
	opTable[0x89] = {"BIT", &CPU::BIT, AddMode::Immediate, 2};
	opTable[0x8A] = {"TXA", &CPU::TXA, AddMode::Implied, 2};
	opTable[0x8B] = {"PHB", &CPU::PHB, AddMode::Implied, 3};
	opTable[0x8C] = {"STY", &CPU::STY, AddMode::Absolute, 4};
	opTable[0x8D] = {"STA", &CPU::STA, AddMode::Absolute, 4};
	opTable[0x8E] = {"STX", &CPU::STX, AddMode::Absolute, 4};
	opTable[0x8F] = {"STA", &CPU::STA, AddMode::AbsoluteLong, 5};

	opTable[0x90] = {"BCC", &CPU::BCC, AddMode::Implied, 2};
	opTable[0x91] = {"STA", &CPU::STA, AddMode::DirectPageIndirectIndexedY, 6};
	opTable[0x92] = {"STA", &CPU::STA, AddMode::DirectPageIndirect, 5};
	opTable[0x93] = {"STA", &CPU::STA, AddMode::StackRelativeIndirectIndexedY, 7};
	opTable[0x94] = {"STY", &CPU::STY, AddMode::DirectPageIndexedX, 4};
	opTable[0x95] = {"STA", &CPU::STA, AddMode::DirectPageIndexedX, 4};
	opTable[0x96] = {"STX", &CPU::STX, AddMode::DirectPageIndexedY, 4};
	opTable[0x97] = {"STA", &CPU::STA, AddMode::DirectPageIndirectLongIndexedY, 6};
	opTable[0x98] = {"TYA", &CPU::TYA, AddMode::Implied, 2};
	opTable[0x99] = {"STA", &CPU::STA, AddMode::AbsoluteIndexedY, 5};
	opTable[0x9A] = {"TXS", &CPU::TXS, AddMode::Implied, 2};
	opTable[0x9B] = {"TXY", &CPU::TXY, AddMode::Implied, 2};
	opTable[0x9C] = {"STZ", &CPU::STZ, AddMode::Absolute, 4};
	opTable[0x9D] = {"STA", &CPU::STA, AddMode::AbsoluteIndexedX, 5};
	opTable[0x9E] = {"STZ", &CPU::STZ, AddMode::AbsoluteIndexedX, 5};
	opTable[0x9F] = {"STA", &CPU::STA, AddMode::AbsoluteLongIndexedX, 5};

	opTable[0xA0] = {"LDY", &CPU::LDY, AddMode::Immediate, 2};
	opTable[0xA1] = {"LDA", &CPU::LDA, AddMode::DirectPageIndirectX, 6};
	opTable[0xA2] = {"LDX", &CPU::LDX, AddMode::Immediate, 2};
	opTable[0xA3] = {"LDA", &CPU::LDA, AddMode::StackRelative, 4};
	opTable[0xA4] = {"LDY", &CPU::LDY, AddMode::DirectPage, 3};
	opTable[0xA5] = {"LDA", &CPU::LDA, AddMode::DirectPage, 3};
	opTable[0xA6] = {"LDX", &CPU::LDX, AddMode::DirectPage, 3};
	opTable[0xA7] = {"LDA", &CPU::LDA, AddMode::DirectPageIndirectLong, 6};
	opTable[0xA8] = {"TAY", &CPU::TAY, AddMode::Implied, 2};
	opTable[0xA9] = {"LDA", &CPU::LDA, AddMode::Immediate, 2};
	opTable[0xAA] = {"TAX", &CPU::TAX, AddMode::Implied, 2};
	opTable[0xAB] = {"PLB", &CPU::PLB, AddMode::Implied, 4};
	opTable[0xAC] = {"LDY", &CPU::LDY, AddMode::Absolute, 4};
	opTable[0xAD] = {"LDA", &CPU::LDA, AddMode::Absolute, 4};
	opTable[0xAE] = {"LDX", &CPU::LDX, AddMode::Absolute, 4};
	opTable[0xAF] = {"LDA", &CPU::LDA, AddMode::AbsoluteLong, 5};

	opTable[0xB0] = {"BCS", &CPU::BCS, AddMode::Implied, 2};
	opTable[0xB1] = {"LDA", &CPU::LDA, AddMode::DirectPageIndirectIndexedY, 5};
	opTable[0xB2] = {"LDA", &CPU::LDA, AddMode::DirectPageIndirect, 5};
	opTable[0xB3] = {"LDA", &CPU::LDA, AddMode::StackRelativeIndirectIndexedY, 7};
	opTable[0xB4] = {"LDY", &CPU::LDY, AddMode::DirectPageIndexedX, 4};
	opTable[0xB5] = {"LDA", &CPU::LDA, AddMode::DirectPageIndexedX, 4};
	opTable[0xB6] = {"LDX", &CPU::LDX, AddMode::DirectPageIndexedY, 4};
	opTable[0xB7] = {"LDA", &CPU::LDA, AddMode::DirectPageIndirectLongIndexedY, 6};
	opTable[0xB8] = {"CLV", &CPU::CLV, AddMode::Implied, 2};
	opTable[0xB9] = {"LDA", &CPU::LDA, AddMode::AbsoluteIndexedY, 4};
	opTable[0xBA] = {"TSX", &CPU::TSX, AddMode::Implied, 2};
	opTable[0xBB] = {"TYX", &CPU::TYX, AddMode::Implied, 2};
	opTable[0xBC] = {"LDY", &CPU::LDY, AddMode::AbsoluteIndexedX, 4};
	opTable[0xBD] = {"LDA", &CPU::LDA, AddMode::AbsoluteIndexedX, 4};
	opTable[0xBE] = {"LDX", &CPU::LDX, AddMode::AbsoluteIndexedY, 4};
	opTable[0xBF] = {"LDA", &CPU::LDA, AddMode::AbsoluteLongIndexedX, 5};

	opTable[0xC0] = {"CPY", &CPU::CPY, AddMode::Immediate, 2};
	opTable[0xC1] = {"CMP", &CPU::CMP, AddMode::DirectPageIndirectX, 6};
	opTable[0xC2] = {"REP", &CPU::REP, AddMode::Immediate, 3};
	opTable[0xC3] = {"CMP", &CPU::CMP, AddMode::StackRelative, 4};
	opTable[0xC4] = {"CPY", &CPU::CPY, AddMode::DirectPage, 3};
	opTable[0xC5] = {"CMP", &CPU::CMP, AddMode::DirectPage, 3};
	opTable[0xC6] = {"DEC", &CPU::DEC, AddMode::DirectPage, 5};
	opTable[0xC7] = {"CMP", &CPU::CMP, AddMode::DirectPageIndirectLong, 6};
	opTable[0xC8] = {"INY", &CPU::INY, AddMode::Implied, 2};
	opTable[0xC9] = {"CMP", &CPU::CMP, AddMode::Immediate, 2};
	opTable[0xCA] = {"DEX", &CPU::DEX, AddMode::Implied, 2};
	opTable[0xCB] = {"WAI", &CPU::WAI, AddMode::Implied, 3};
	opTable[0xCC] = {"CPY", &CPU::CPY, AddMode::Absolute, 4};
	opTable[0xCD] = {"CMP", &CPU::CMP, AddMode::Absolute, 4};
	opTable[0xCE] = {"DEC", &CPU::DEC, AddMode::Absolute, 6};
	opTable[0xCF] = {"CMP", &CPU::CMP, AddMode::AbsoluteLong, 5};

	opTable[0xD0] = {"BNE", &CPU::BNE, AddMode::Implied, 2};
	opTable[0xD1] = {"CMP", &CPU::CMP, AddMode::DirectPageIndirectIndexedY, 5};
	opTable[0xD2] = {"CMP", &CPU::CMP, AddMode::DirectPageIndirect, 5};
	opTable[0xD3] = {"CMP", &CPU::CMP, AddMode::StackRelativeIndirectIndexedY, 7};
	opTable[0xD4] = {"PEI", &CPU::PEI, AddMode::Implied, 6};
	opTable[0xD5] = {"CMP", &CPU::CMP, AddMode::DirectPageIndexedX, 4};
	opTable[0xD6] = {"DEC", &CPU::DEC, AddMode::DirectPageIndexedX, 6};
	opTable[0xD7] = {"CMP", &CPU::CMP, AddMode::DirectPageIndirectLongIndexedY, 6};
	opTable[0xD8] = {"CLD", &CPU::CLD, AddMode::Implied, 2};
	opTable[0xD9] = {"CMP", &CPU::CMP, AddMode::AbsoluteIndexedY, 4};
	opTable[0xDA] = {"PHX", &CPU::PHX, AddMode::Implied, 3};
	opTable[0xDB] = {"STP", &CPU::STP, AddMode::Implied, 3};
	opTable[0xDC] = {"JML", &CPU::JML, AddMode::AbsoluteIndirectLong, 6};
	opTable[0xDD] = {"CMP", &CPU::CMP, AddMode::AbsoluteIndexedX, 4};
	opTable[0xDE] = {"DEC", &CPU::DEC, AddMode::AbsoluteIndexedX, 7};
	opTable[0xDF] = {"CMP", &CPU::CMP, AddMode::AbsoluteLongIndexedX, 5};

	opTable[0xE0] = {"CPX", &CPU::CPX, AddMode::Immediate, 2};
	opTable[0xE1] = {"SBC", &CPU::SBC, AddMode::DirectPageIndirectX, 6};
	opTable[0xE2] = {"SEP", &CPU::SEP, AddMode::Immediate, 3};
	opTable[0xE3] = {"SBC", &CPU::SBC, AddMode::StackRelative, 4};
	opTable[0xE4] = {"CPX", &CPU::CPX, AddMode::DirectPage, 3};
	opTable[0xE5] = {"SBC", &CPU::SBC, AddMode::DirectPage, 3};
	opTable[0xE6] = {"INC", &CPU::INC, AddMode::DirectPage, 5};
	opTable[0xE7] = {"SBC", &CPU::SBC, AddMode::DirectPageIndirectLong, 6};
	opTable[0xE8] = {"INX", &CPU::INX, AddMode::Implied, 2};
	opTable[0xE9] = {"SBC", &CPU::SBC, AddMode::Immediate, 2};
	opTable[0xEA] = {"NOP", &CPU::NOP, AddMode::Implied, 2};
	opTable[0xEB] = {"XBA", &CPU::XBA, AddMode::Implied, 3};
	opTable[0xEC] = {"CPX", &CPU::CPX, AddMode::Absolute, 4};
	opTable[0xED] = {"SBC", &CPU::SBC, AddMode::Absolute, 4};
	opTable[0xEE] = {"INC", &CPU::INC, AddMode::Absolute, 6};
	opTable[0xEF] = {"SBC", &CPU::SBC, AddMode::AbsoluteLong, 5};

	opTable[0xF0] = {"BEQ", &CPU::BEQ, AddMode::Implied, 2};
	opTable[0xF1] = {"SBC", &CPU::SBC, AddMode::DirectPageIndirectIndexedY, 5};
	opTable[0xF2] = {"SBC", &CPU::SBC, AddMode::DirectPageIndirect, 5};
	opTable[0xF3] = {"SBC", &CPU::SBC, AddMode::StackRelativeIndirectIndexedY, 7};
	opTable[0xF4] = {"PEA", &CPU::PEA, AddMode::Implied, 5};
	opTable[0xF5] = {"SBC", &CPU::SBC, AddMode::DirectPageIndexedX, 4};
	opTable[0xF6] = {"INC", &CPU::INC, AddMode::DirectPageIndexedX, 6};
	opTable[0xF7] = {"SBC", &CPU::SBC, AddMode::DirectPageIndirectLongIndexedY, 6};
	opTable[0xF8] = {"SED", &CPU::SED, AddMode::Implied, 2};
	opTable[0xF9] = {"SBC", &CPU::SBC, AddMode::AbsoluteIndexedY, 4};
	opTable[0xFA] = {"PLX", &CPU::PLX, AddMode::Implied, 4};
	opTable[0xFB] = {"XCE", &CPU::XCE, AddMode::Implied, 2};
	opTable[0xFC] = {"JSR", &CPU::JSR, AddMode::AbsoluteIndexedIndirectX, 8};
	opTable[0xFD] = {"SBC", &CPU::SBC, AddMode::AbsoluteIndexedX, 4};
	opTable[0xFE] = {"INC", &CPU::INC, AddMode::AbsoluteIndexedX, 7};
	opTable[0xFF] = {"SBC", &CPU::SBC, AddMode::AbsoluteLongIndexedX, 5};
}

int CPU::Clock() {
	int opcycles = 0;
	if (cycles == 0) {
		if (startDMA) {
			opcycles = DMA();
			cycles = opcycles;
		}
		else if (CheckIRQ()) {
			IRQ();
		}
		else {
			opcode = ReadMemory((regs.PB << 16) | regs.PC);
	
			if (pendingDMA) {
				startDMA = true;
				pendingDMA = false;
			}
			//printf("\033[0;36mPC: 0x%02x%04x OP: %s %02x\033[0m\n", regs.PB, regs.PC, opTable[opcode].name, opcode);
			//Debug();
	
			regs.PC++;
	
			opcycles = opTable[opcode].cycles;
			opcycles += (this->*opTable[opcode].execute)();
			cycles = opcycles;
		}
	}
	cycles--;
	return opcycles;
}

uint8_t CPU::ReadMemory(uint32_t address) {
	return memory->ReadMemory(address);
}

void CPU::WriteMemory(uint32_t address, uint8_t value) {
	memory->WriteMemory(address, value);
}

void CPU::SetExcVectors(uint16_t vectors[16]) {
	excVectors.COPVector = vectors[2];
	excVectors.BRKVector = vectors[3];
	excVectors.NMIVector = vectors[5];
	excVectors.IRQVector = vectors[7];

	excVectors.ResetVector = vectors[14];

	regs.PC = excVectors.ResetVector;
}

void CPU::NMI() {
	nmi |= 0x80;

	if (NMITIMEN & 0x80) {
		PushStack(regs.PB);

		PushStack(regs.PC >> 8);
		PushStack(regs.PC & 0xFF);

		PushStack(regs.P);

		regs.P |= IFlag;
		regs.P &= ~DFlag;

		regs.PB = 0;

		regs.PC = excVectors.NMIVector;

		cycles += 8;
		// TODO: Check if Waiting
	}
}

bool CPU::CheckIRQ() {
	// IRQ Interrupts disable
	if (regs.P & IFlag) {
		return false;
	}

	// IRQ Interrupts enable
	if (NMITIMEN & 0x30) {
		bool VCheck = NMITIMEN & 0x20;
		bool HCheck = NMITIMEN & 0x10;

		if (VCheck && HCheck) {
			if (memory->ppu->VCounter == VTIME) {
				if (memory->ppu->HCounter >= HTIME) {
					return true;
				}
			}
		}
		else if (VCheck) {
			if (memory->ppu->VCounter == VTIME) {
				return true;
			}
		}
		else if (HCheck) {
			if (memory->ppu->HCounter == HTIME) {
				return true;
			}
		}
	}
	return false;
}

void CPU::IRQ() {
	TIMEUP = true;
	PushStack(regs.PB);

	PushStack(regs.PC >> 8);
	PushStack(regs.PC & 0xFF);

	PushStack(regs.P);

	regs.P |= IFlag;
	regs.P &= ~DFlag;

	regs.PB = 0;

	regs.PC = excVectors.IRQVector;

	cycles += 8;
}

uint8_t CPU::ReadCPU(uint32_t address) {
	uint8_t value = 0;

	uint8_t page = (address & 0xFF00) >> 8;
	uint8_t reg = address & 0xFF;

	if (page == 0x42) {
		if (reg == 0x10) {
			nmi |= 0x02;
			value = nmi;
			nmi &= ~0x80;
			return value;
		}
		else if (reg == 0x11) {
			value = TIMEUP ? 0x80 : 0x00;
			TIMEUP = false;
		}
		else if (reg == 0x12) {
			value |= (memory->ppu->VCounter >= 225) ? 0x80 : 0x00;
			value |= (memory->ppu->HCounter >= 0x121) ? 0x40 : 0x00;
			// Bit 0 is Auto-Joypad Status
			return value;
		}
		else if (reg == 0x18) {
			return joypad1Low;
		}
		else if (reg == 0x19) {
			return joypad1High;
		}
	}
	printf("\033[0;36mReading CPU: %06x\033[0m\n", address);
	//TODO: CPU Reading
	return value;
}

void CPU::WriteCPU(uint32_t address, uint8_t value) {
	uint8_t page = (address & 0xFF00) >> 8;
	uint8_t reg = address & 0xFF;

	if (page == 0x42) {
		if (reg == 0x00) {		// NMITIMEN
			NMITIMEN = value;
		}
		else if (reg == 0x07) {	// HTIMEL
			HTIME &= 0xFF00;
			HTIME |= value;
		}
		else if (reg == 0x08) {	// HTIMEH
			HTIME &= 0x00FF;
			HTIME |= value << 8;
		}
		else if (reg == 0x09) {	// VTIMEL
			VTIME &= 0xFF00;
			VTIME |= value;
		}
		else if (reg == 0x0A) {	// VTIMEH
			VTIME &= 0x00FF;
			VTIME |= value << 8;
		}
		else if (reg == 0x0B) {	// MDMAEN
			if (value != 0) {
				enabledDMA = value;
				pendingDMA = true;
				printf("\033[0;36mStarting DMA\033[0m\n");
			}
		}
		else if (reg == 0x0C) {	// HDMAEN
			if (value != 0) {
				enabledHDMA = value;
			}
		}
		else if (reg == 0x0D) {	// MEMSEL
			if (value & 0x1) {
				printf("Fast ROM\n");
				return;
			} else {
				printf("Slow ROM\n");
				return;
			}
		}
		else {
			printf("\033[0;36mWritting CPU: %06x = %02x\033[0m\n", address, value);
			//TODO: CPU Writting
		}
	}
	else if (page == 0x43) {
		uint8_t dmaChannel = (reg & 0xF0) >> 4;
		reg &= 0xF;

		if (dmaChannel < 8 && reg < 11) {
			if (reg == 0x0) {		// DMAPx
				Dma[dmaChannel].direction = value & 0x80;
				Dma[dmaChannel].indirect = value & 0x40;
				Dma[dmaChannel].increment = (value >> 3) & 0x03;
				Dma[dmaChannel].mode = value & 0x7;
			}
			else if (reg == 0x1) {	// BBADx
				Dma[dmaChannel].BAddress = 0x2100 | value;
			}
			else if (reg == 0x2) {	// A1TxL
				Dma[dmaChannel].AAddress &= 0xFF00;
				Dma[dmaChannel].AAddress |= value;
			}
			else if (reg == 0x3) {	// A1TxH
				Dma[dmaChannel].AAddress &= 0x00FF;
				Dma[dmaChannel].AAddress |= (value << 8);
			}
			else if (reg == 0x4) {	// A1Bx
				Dma[dmaChannel].ABank = value;
			}
			else if (reg == 0x5) {	// DASxL
				Dma[dmaChannel].count &= 0xFFFF00;
				Dma[dmaChannel].count |= value;
			}
			else if (reg == 0x6) {	// DASxH
				Dma[dmaChannel].count &= 0xFF00FF;
				Dma[dmaChannel].count |= (value << 8);
			}
			else if (reg == 0x7) {	// DASBx
				Dma[dmaChannel].count &= 0x00FFFF;
				Dma[dmaChannel].count |= (value << 16);
			}
			else if (reg == 0x8) {	// A2AxL
				Dma[dmaChannel].tableAddress &= 0xFF00;
				Dma[dmaChannel].tableAddress |= value;
			}
			else if (reg == 0x9) {	// A2AxH
				Dma[dmaChannel].tableAddress &= 0x00FF;
				Dma[dmaChannel].tableAddress |= (value << 8);
			}
			else if (reg == 0xA) {	// NTRLx
				Dma[dmaChannel].repeat = value & 0x80;
				Dma[dmaChannel].lineCounter = value & 0x7F;
			}
		}
		else {
			printf("\033[0;36mWritting CPU: %06x = %02x\033[0m\n", address, value);
			//TODO: CPU Writting
		}
	}
}

void CPU::StartHDMA() {
	for (uint8_t i = 0; i < 8; i++) {
		if ((enabledHDMA & (1 << i)) && !Dma[i].finished) {
			if (--Dma[i].lineCounter != 0) {
				continue;
			}

			uint8_t tableEntry = ReadMemory(Dma[i].tableAddress | (Dma[i].ABank << 16));
			Dma[i].repeat = tableEntry & 0x80;
			Dma[i].lineCounter = tableEntry & 0x7F;
			Dma[i].tableAddress++;
			if (Dma[i].indirect) {
				assert(false && "HDMA Indirect");
			} else {
				switch (Dma[i].mode) {
					case 0:
					{
						if (!Dma[i].direction) {
							WriteMemory(Dma[i].BAddress, ReadMemory(Dma[i].tableAddress | (Dma[i].ABank << 16)));
						} else {
							WriteMemory(Dma[i].tableAddress | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress));
						}
						Dma[i].tableAddress++;
						break;
					}

					case 1:
					{
						if (!Dma[i].direction) {
							WriteMemory(Dma[i].BAddress, ReadMemory(Dma[i].tableAddress | (Dma[i].ABank << 16)));
							WriteMemory(Dma[i].BAddress + 1, ReadMemory(Dma[i].tableAddress + 1 | (Dma[i].ABank << 16)));
						} else {
							WriteMemory(Dma[i].tableAddress | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress));
							WriteMemory(Dma[i].tableAddress + 1 | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress + 1));
						}
						Dma[i].tableAddress += 2;
						break;
					}

					case 2:
					{
						if (!Dma[i].direction) {
							WriteMemory(Dma[i].BAddress, ReadMemory(Dma[i].tableAddress | (Dma[i].ABank << 16)));
							WriteMemory(Dma[i].BAddress, ReadMemory(Dma[i].tableAddress + 1 | (Dma[i].ABank << 16)));
						} else {
							WriteMemory(Dma[i].tableAddress | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress));
							WriteMemory(Dma[i].tableAddress | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress + 1));
						}
						Dma[i].tableAddress += 2;
						break;
					}

					case 3:
					{
						if (!Dma[i].direction) {
							WriteMemory(Dma[i].BAddress, ReadMemory(Dma[i].tableAddress | (Dma[i].ABank << 16)));
							WriteMemory(Dma[i].BAddress, ReadMemory(Dma[i].tableAddress + 1 | (Dma[i].ABank << 16)));
							WriteMemory(Dma[i].BAddress + 1, ReadMemory(Dma[i].tableAddress + 2 | (Dma[i].ABank << 16)));
							WriteMemory(Dma[i].BAddress + 1, ReadMemory(Dma[i].tableAddress + 3 | (Dma[i].ABank << 16)));
						} else {
							WriteMemory(Dma[i].tableAddress | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress));
							WriteMemory(Dma[i].tableAddress | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress + 1));
							WriteMemory(Dma[i].tableAddress + 1 | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress + 2));
							WriteMemory(Dma[i].tableAddress + 1 | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress + 3));
						}
						Dma[i].tableAddress += 4;
						break;
					}

					default:
						assert(false && "DMA Mode not handled");
				}
			}
			if (ReadMemory(Dma[i].tableAddress | (Dma[i].ABank << 16)) == 0) {
				Dma[i].finished = true;
			}
		}
	}
}

void CPU::ResetHDMA() {
	for (uint8_t i = 0; i < 8; i++) {
		if (enabledHDMA & (1 << i)) {
			Dma[i].finished = false;
			Dma[i].tableAddress = Dma[i].AAddress;
			uint8_t tableEntry = ReadMemory(Dma[i].tableAddress | (Dma[i].ABank << 16));
			Dma[i].repeat = tableEntry & 0x80;
			Dma[i].lineCounter = tableEntry & 0x7F;
			Dma[i].tableAddress++;

			if (Dma[i].indirect) {
				assert(false && "HDMA Indirect");
			} else {
				switch (Dma[i].mode) {
					case 0:
					{
						if (!Dma[i].direction) {
							WriteMemory(Dma[i].BAddress, ReadMemory(Dma[i].tableAddress | (Dma[i].ABank << 16)));
						} else {
							WriteMemory(Dma[i].tableAddress | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress));
						}
						Dma[i].tableAddress++;
						break;
					}

					case 1:
					{
						if (!Dma[i].direction) {
							WriteMemory(Dma[i].BAddress, ReadMemory(Dma[i].tableAddress | (Dma[i].ABank << 16)));
							WriteMemory(Dma[i].BAddress + 1, ReadMemory(Dma[i].tableAddress + 1 | (Dma[i].ABank << 16)));
						} else {
							WriteMemory(Dma[i].tableAddress | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress));
							WriteMemory(Dma[i].tableAddress + 1 | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress + 1));
						}
						Dma[i].tableAddress += 2;
						break;
					}

					case 2:
					{
						if (!Dma[i].direction) {
							WriteMemory(Dma[i].BAddress, ReadMemory(Dma[i].tableAddress | (Dma[i].ABank << 16)));
							WriteMemory(Dma[i].BAddress, ReadMemory(Dma[i].tableAddress + 1 | (Dma[i].ABank << 16)));
						} else {
							WriteMemory(Dma[i].tableAddress | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress));
							WriteMemory(Dma[i].tableAddress | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress + 1));
						}
						Dma[i].tableAddress += 2;
						break;
					}

					case 3:
					{
						if (!Dma[i].direction) {
							WriteMemory(Dma[i].BAddress, ReadMemory(Dma[i].tableAddress | (Dma[i].ABank << 16)));
							WriteMemory(Dma[i].BAddress, ReadMemory(Dma[i].tableAddress + 1 | (Dma[i].ABank << 16)));
							WriteMemory(Dma[i].BAddress + 1, ReadMemory(Dma[i].tableAddress + 2 | (Dma[i].ABank << 16)));
							WriteMemory(Dma[i].BAddress + 1, ReadMemory(Dma[i].tableAddress + 3 | (Dma[i].ABank << 16)));
						} else {
							WriteMemory(Dma[i].tableAddress | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress));
							WriteMemory(Dma[i].tableAddress | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress + 1));
							WriteMemory(Dma[i].tableAddress + 1 | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress + 2));
							WriteMemory(Dma[i].tableAddress + 1 | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress + 3));
						}
						Dma[i].tableAddress += 4;
						break;
					}

					default:
						printf("Mode: %d\n", Dma[i].mode);
						assert(false && "DMA Mode not handled");
				}
			}
		}
	}
}

int CPU::DMA() {
	int extraCycles = 0;
	for (uint8_t i = 0; i < 8; i++) {
		if (enabledDMA & (1 << i)) {
			extraCycles += 1;
			
			do {
				switch (Dma[i].mode) {
					case 0:
					{
						if (!Dma[i].direction) {
							WriteMemory(Dma[i].BAddress, ReadMemory(Dma[i].AAddress | (Dma[i].ABank << 16)));
						} else {
							WriteMemory(Dma[i].AAddress | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress));
						}
						extraCycles += 1;

						Dma[i].count--;
						Dma[i].count &= 0xFFFF;
						if (Dma[i].count == 0) {
							enabledDMA &= ~(1 << i);
							if (enabledDMA == 0x00) {
								extraCycles += 1;
								startDMA = false;
								printf("\033[0;36mEnding DMA\033[0m\n");
								return extraCycles;
							}
						}
						Dma[i].AAddress += (Dma[i].increment == 0) ? 1 : ((Dma[i].increment == 2) ? -1 : 0);
						break;
					}

					case 1:
					{
						if (!Dma[i].direction) {
							WriteMemory(Dma[i].BAddress, ReadMemory(Dma[i].AAddress | (Dma[i].ABank << 16)));
							if (--Dma[i].count == 0) {
								extraCycles += 1;
								enabledDMA &= ~(1 << i);
								if (enabledDMA == 0x00) {
									extraCycles += 1;
									startDMA = false;
									printf("\033[0;36mEnding DMA\033[0m\n");
									return extraCycles;
								}
							}
							WriteMemory(Dma[i].BAddress + 1, ReadMemory(Dma[i].AAddress + 1 | (Dma[i].ABank << 16)));
						} else {
							WriteMemory(Dma[i].AAddress | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress));
							if (--Dma[i].count == 0) {
								extraCycles += 1;
								enabledDMA &= ~(1 << i);
								if (enabledDMA == 0x00) {
									extraCycles += 1;
									startDMA = false;
									printf("\033[0;36mEnding DMA\033[0m\n");
									return extraCycles;
								}
							}
							WriteMemory(Dma[i].AAddress + 1 | (Dma[i].ABank << 16), ReadMemory(Dma[i].BAddress + 1));
						}
						extraCycles += 1;

						Dma[i].count--;
						Dma[i].count &= 0xFFFF;
						if (Dma[i].count == 0) {
							enabledDMA &= ~(1 << i);
							if (enabledDMA == 0x00) {
								extraCycles += 1;
								startDMA = false;
								printf("\033[0;36mEnding DMA\033[0m\n");
								return extraCycles;
							}
						}
						Dma[i].AAddress += (Dma[i].increment == 0) ? 2 : ((Dma[i].increment == 2) ? -2 : 0);
						break;
					}
					default:
						assert(false && "DMA Mode not handled");
				}
			} while (Dma[i].count != 0);
			extraCycles += 1;
			break;
		}
	}
	return extraCycles;
}

bool enableLog = false;
FILE* logfile = nullptr;
int logfileMaxSize = 7277;
int logfileLineCount = 0;
void CPU::Debug() {
	if (logfile == nullptr && enableLog) logfile = fopen("/home/legolasbat/Descargas/SMW_log.txt", "w");
	if (logfile) {
		fprintf(logfile, "%02x%04x %s A:%04x X:%04x Y:%04x S:%04x D:%04x DB:%02x %s%s%s%s%s%s%s%s V:%3d H:%3d F:%2d\n",
				regs.PB, regs.PC, opTable[opcode].name, regs.A.HL, regs.X.HL, regs.Y.HL, regs.SP, regs.DP, regs.DB,
				(regs.P & NFlag) ? "N" : "n",
				(regs.P & VFlag) ? "V" : "v",
				emulationMode ? "1" : (regs.P & MFlag) ? "M" : "m",
				emulationMode ? "B" : (regs.P & XFlag) ? "X" : "x",
				(regs.P & DFlag) ? "D" : "d",
				(regs.P & IFlag) ? "I" : "i",
				(regs.P & ZFlag) ? "Z" : "z",
				(regs.P & CFlag) ? "C" : "c",
				memory->ppu->VCounter, memory->ppu->HCounter, memory->ppu->frameCounter);
		fflush(logfile);
		//logfileLineCount++;
		if (!enableLog) {
			fclose(logfile);
			assert(false);
		}
	} else {
		printf("%02x%04x %s A:%04x X:%04x Y:%04x S:%04x D:%04x DB:%02x %s%s%s%s%s%s%s%s V:%3d H:%3d F:%2d\n",
			regs.PB, regs.PC, opTable[opcode].name, regs.A.HL, regs.X.HL, regs.Y.HL, regs.SP, regs.DP, regs.DB,
			(regs.P & NFlag) ? "N" : "n",
			(regs.P & VFlag) ? "V" : "v",
			emulationMode ? "1" : (regs.P & MFlag) ? "M" : "m",
			emulationMode ? "B" : (regs.P & XFlag) ? "X" : "x",
			(regs.P & DFlag) ? "D" : "d",
			(regs.P & IFlag) ? "I" : "i",
			(regs.P & ZFlag) ? "Z" : "z",
			(regs.P & CFlag) ? "C" : "c",
			memory->ppu->VCounter, memory->ppu->HCounter, memory->ppu->frameCounter);
	}
}

void CPU::SetFlag(uint8_t flag, bool state) {
	if (state) {
		regs.P |= flag;
	} else {
		regs.P &= ~flag;
	}
}

void CPU::CheckNFlag8Bits(uint8_t value) {
	SetFlag(NFlag, value & 0x80);
}

void CPU::CheckZFlag8Bits(uint8_t value) {
	SetFlag(ZFlag, !value);
}

void CPU::CheckNFlag16Bits(uint16_t value) {
	SetFlag(NFlag, value & 0x8000);
}

void CPU::CheckZFlag16Bits(uint16_t value) {
	SetFlag(ZFlag, !value);
}

uint8_t CPU::PullStack() {
	return ReadMemory(++regs.SP);
}

void CPU::PushStack(uint8_t value) {
	WriteMemory(regs.SP--, value);
}

// Addressing Modes

uint8_t CPU::FetchValue(AddMode addMode, bool is16Bits) {
	switch (addMode) {
		case AddMode::Implied:
			return IMP(is16Bits);

		case AddMode::Immediate:
			return IMM(is16Bits);

		case AddMode::Absolute:
		{
			uint8_t extraCycles = ABS();
			uint32_t fullAddress = (regs.DB << 16) | fetchAddress;
			fetched = ReadMemory(fullAddress);
			if (is16Bits) {
				extraCycles++;
				fetched |= ReadMemory((fullAddress + 1) & 0xFFFFFF) << 8;
			}
			return extraCycles;
		}

		case AddMode::AbsoluteLong:
		{
			uint8_t extraCycles = ABSLong();
			fetched = ReadMemory(fetchAddress);
			if (is16Bits) {
				extraCycles++;
				fetched |= ReadMemory((fetchAddress + 1) & 0xFFFFFF) << 8;
			}
			return extraCycles;
		}

		case AddMode::AbsoluteLongIndexedX:
		{
			uint8_t extraCycles = ABSLongIndexX();
			fetched = ReadMemory(fetchAddress);
			if (is16Bits) {
				extraCycles++;
				fetched |= ReadMemory(fetchAddress + 1) << 8;
			}
			return extraCycles;
		}

		case AddMode::AbsoluteIndexedX:
		{
			uint8_t extraCycles = ABSIndexX();
			uint32_t fullAddress = (regs.DB << 16) | fetchAddress;
			fetched = ReadMemory(fullAddress);
			if (is16Bits) {
				extraCycles++;
				fetched |= ReadMemory(fullAddress + 1) << 8;
			}
			return extraCycles;
		}

		case AddMode::AbsoluteIndexedY:
		{
			uint8_t extraCycles = ABSIndexY();
			uint32_t fullAddress = (regs.DB << 16) | fetchAddress;
			fetched = ReadMemory(fullAddress);
			if (is16Bits) {
				extraCycles++;
				fetched |= ReadMemory(fullAddress + 1) << 8;
			}
			return extraCycles;
		}

		case AddMode::DirectPage:
		{
			uint8_t extraCycles = DirectPage();
			
			fetched = ReadMemory(fetchAddress);
			if (is16Bits) {
				extraCycles++;
				fetched |= ReadMemory((fetchAddress + 1) & 0xFFFF) << 8;
			}
			return extraCycles;
		}

		case AddMode::DirectPageIndexedX:
		{
			uint8_t extraCycles = DirectPageIndexedX();
			
			fetched = ReadMemory(fetchAddress);
			if (is16Bits) {
				extraCycles++;
				fetched |= ReadMemory((fetchAddress + 1) & 0xFFFF) << 8;
			}
			return extraCycles;
		}

		case AddMode::DirectPageIndexedY:
		{
			uint8_t extraCycles = DirectPageIndexedY();
			
			fetched = ReadMemory(fetchAddress);
			if (is16Bits) {
				extraCycles++;
				fetched |= ReadMemory((fetchAddress + 1) & 0xFFFF) << 8;
			}
			return extraCycles;
		}
		
		case AddMode::DirectPageIndirect:
		{
			uint8_t extraCycles = DirectPageIndirect();
			uint32_t fullAddress = (regs.DB << 16) | fetchAddress;
			fetched = ReadMemory(fullAddress);
			if (is16Bits) {
				extraCycles++;
				fetched |= ReadMemory((fullAddress + 1) & 0xFFFFFF) << 8;
			}
			return extraCycles;
		}
		
		case AddMode::DirectPageIndirectX:
		{
			uint8_t extraCycles = DirectPageIndirectX();
			uint32_t fullAddress = (regs.DB << 16) | fetchAddress;
			fetched = ReadMemory(fullAddress);
			if (is16Bits) {
				extraCycles++;
				fetched |= ReadMemory((fullAddress + 1) & 0xFFFFFF) << 8;
			}
			return extraCycles;
		}

		case AddMode::DirectPageIndirectIndexedY:
		{
			uint8_t extraCycles = DirectPageIndirectIndexedY();
			uint32_t fullAddress = (regs.DB << 16) | fetchAddress;
			fetched = ReadMemory(fullAddress);
			if (is16Bits) {
				extraCycles++;
				fetched |= ReadMemory((fullAddress + 1) & 0xFFFFFF) << 8;
			}
			return extraCycles;
		}

		case AddMode::DirectPageIndirectLong:
		{
			uint8_t extraCycles = DirectPageIndirectLong();
			fetched = ReadMemory(fetchAddress);
			if (is16Bits) {
				extraCycles++;
				fetched |= ReadMemory((fetchAddress + 1) & 0xFFFFFF) << 8;
			}
			return extraCycles;
		}
		
		case AddMode::DirectPageIndirectLongIndexedY:
		{
			uint8_t extraCycles = DirectPageIndirectLongIndexedY();
			fetched = ReadMemory(fetchAddress);
			if (is16Bits) {
				extraCycles++;
				fetched |= ReadMemory((fetchAddress + 1) & 0xFFFFFF) << 8;
			}
			return extraCycles;
		}

		case AddMode::StackRelative:
		{
			uint8_t extraCycles = StackRel();
			fetched = ReadMemory(fetchAddress);
			if (is16Bits) {
				extraCycles++;
				fetched |= ReadMemory((fetchAddress + 1) & 0xFFFF) << 8;
			}
			return extraCycles;
		}

		case AddMode::StackRelativeIndirectIndexedY:
		{
			uint8_t extraCycles = StackRelativeIndirectIndexedY();
			fetched = ReadMemory(fetchAddress);
			if (is16Bits) {
				fetched |= ReadMemory((fetchAddress + 1) & 0xFFFFFF) << 8;
			}
			return extraCycles;
		}

		default:
			assert(false && "No read addressing mode");
			return 0;
	}
}


uint8_t CPU::WriteValue(AddMode addMode, uint16_t value, bool is16Bits, bool read) {
	switch (addMode) {
		case AddMode::Implied:
		{
			if (is16Bits) {
				regs.A.HL = value;
			} else {
				regs.A.L = value;
			}
			return 0;
		}

		case AddMode::Absolute:
		{
			uint8_t extraCycles = 0;
			if (!read) {
				extraCycles = ABS();
			}
			uint32_t fullAddress = (regs.DB << 16) | fetchAddress;
			WriteMemory(fullAddress, value);
			if (is16Bits) {
				extraCycles++;
				WriteMemory((fullAddress + 1) & 0xFFFFFF, value >> 8);
			}
			return extraCycles;
		}

		case AddMode::AbsoluteLong:
		{
			uint8_t extraCycles = 0;
			if (!read) {
				extraCycles = ABSLong();
			}
			WriteMemory(fetchAddress, value);
			if (is16Bits) {
				extraCycles++;
				WriteMemory((fetchAddress + 1) & 0xFFFFFF, value >> 8);
			}
			return extraCycles;
		}

		case AddMode::AbsoluteLongIndexedX:
		{
			uint8_t extraCycles = 0;
			if (!read) {
				extraCycles = ABSLongIndexX();
			}
			WriteMemory(fetchAddress, value);
			if (is16Bits) {
				extraCycles++;
				WriteMemory((fetchAddress + 1) & 0xFFFFFF, value >> 8);
			}
			return extraCycles;
		}

		case AddMode::AbsoluteIndexedX:
		{
			uint8_t extraCycles = 0;
			if (!read) {
				extraCycles = ABSIndexX();
			}
			uint32_t fullAddress = (regs.DB << 16) | fetchAddress;
			WriteMemory(fullAddress, value);
			if (is16Bits) {
				extraCycles++;
				WriteMemory((fullAddress + 1) & 0xFFFFFF, value >> 8);
			}
			return extraCycles;
		}

		case AddMode::AbsoluteIndexedY:
		{
			uint8_t extraCycles = 0;
			if (!read) {
				extraCycles = ABSIndexY();
			}
			uint32_t fullAddress = (regs.DB << 16) | fetchAddress;
			WriteMemory(fullAddress, value);
			if (is16Bits) {
				extraCycles++;
				WriteMemory((fullAddress + 1) & 0xFFFFFF, value >> 8);
			}
			return extraCycles;
		}

		case AddMode::DirectPage:
		{
			uint8_t extraCycles = 0;
			if (!read) {
				extraCycles = DirectPage();
			}
			WriteMemory(fetchAddress, value);
			if (is16Bits) {
				extraCycles++;
				WriteMemory((fetchAddress + 1) & 0xFFFF, value >> 8);
			}
			return extraCycles;
		}

		case AddMode::DirectPageIndexedX:
		{
			uint8_t extraCycles = 0;
			if (!read) {
				extraCycles = DirectPageIndexedX();
			}
			WriteMemory(fetchAddress, value);
			if (is16Bits) {
				extraCycles++;
				WriteMemory((fetchAddress + 1) & 0xFFFF, value >> 8);
			}
			return extraCycles;
		}
	
		case AddMode::DirectPageIndexedY:
		{
			uint8_t extraCycles = 0;
			if (!read) {
				extraCycles = DirectPageIndexedY();
			}
			WriteMemory(fetchAddress, value);
			if (is16Bits) {
				extraCycles++;
				WriteMemory((fetchAddress + 1) & 0xFFFF, value >> 8);
			}
			return extraCycles;
		}

		case AddMode::DirectPageIndirect:
		{
			uint8_t extraCycles = 0;
			if (!read) {
				extraCycles = DirectPageIndirect();
			}
			uint32_t fullAddress = (regs.DB << 16) | fetchAddress;
			WriteMemory(fullAddress, value);
			if (is16Bits) {
				extraCycles++;
				WriteMemory((fullAddress + 1) & 0xFFFFFF, value >> 8);
			}
			return extraCycles;
		}

		case AddMode::DirectPageIndirectX:
		{
			uint8_t extraCycles = 0;
			if (!read) {
				extraCycles = DirectPageIndirectX();
			}
			uint32_t fullAddress = (regs.DB << 16) | fetchAddress;
			WriteMemory(fullAddress, value);
			if (is16Bits) {
				extraCycles++;
				WriteMemory((fullAddress + 1) & 0xFFFFFF, value >> 8);
			}
			return extraCycles;
		}

		case AddMode::DirectPageIndirectIndexedY:
		{
			uint8_t extraCycles = 0;
			if (!read) {
				extraCycles = DirectPageIndirectIndexedY();
			}
			uint32_t fullAddress = (regs.DB << 16) | fetchAddress;
			WriteMemory(fullAddress, value);
			if (is16Bits) {
				extraCycles++;
				WriteMemory((fullAddress + 1) & 0xFFFFFF, value >> 8);
			}
			return extraCycles;
		}

		case AddMode::DirectPageIndirectLong:
		{
			uint8_t extraCycles = 0;
			if (!read) {
				extraCycles = DirectPageIndirectLong();
			}
			WriteMemory(fetchAddress, value);
			if (is16Bits) {
				extraCycles++;
				WriteMemory((fetchAddress + 1) & 0xFFFFFF, value >> 8);
			}
			return extraCycles;
		}

		case AddMode::DirectPageIndirectLongIndexedY:
		{
			uint8_t extraCycles = 0;
			if (!read) {
				extraCycles = DirectPageIndirectLongIndexedY();
			}
			WriteMemory(fetchAddress, value);
			if (is16Bits) {
				extraCycles++;
				WriteMemory((fetchAddress + 1) & 0xFFFFFF, value >> 8);
			}
			return extraCycles;
		}

		case AddMode::StackRelative:
		{
			uint8_t extraCycles = 0;
			if (!read) {
				extraCycles = StackRel();
			}
			WriteMemory(fetchAddress, value);
			if (is16Bits) {
				extraCycles++;
				WriteMemory((fetchAddress + 1) & 0xFFFF, value >> 8);
			}
			return extraCycles;
		}

		case AddMode::StackRelativeIndirectIndexedY:
		{
			uint8_t extraCycles = 0;
			if (!read) {
				extraCycles = StackRelativeIndirectIndexedY();
			}
			WriteMemory(fetchAddress, value);
			if (is16Bits) {
				WriteMemory((fetchAddress + 1) & 0xFFFFFF, value >> 8);
			}
			return extraCycles;
		}

		default:
			assert(false && "No write addressing mode");
			return 0;
	}
}

uint8_t CPU::IMP(bool is16Bits) {
	if (is16Bits) {
		fetched = regs.A.HL;
	} else {
		fetched = regs.A.L;
	}
	return 0;
}

uint8_t CPU::IMM(bool is16Bits) {
	fetched = ReadMemory((regs.PB << 16) | regs.PC++);
	
	if (is16Bits) {
		fetched |= ReadMemory((regs.PB << 16) | regs.PC++) << 8;
		return 1;
	}

	return 0;
}

uint8_t CPU::ABS() {
	fetchAddress = ReadMemory((regs.PB << 16) | regs.PC++);
	fetchAddress |= ReadMemory((regs.PB << 16) | regs.PC++) << 8;

	return 0;
}

uint8_t CPU::ABSLong() {
	fetchAddress = ReadMemory((regs.PB << 16) | regs.PC++);
	fetchAddress |= ReadMemory((regs.PB << 16) | regs.PC++) << 8;
	fetchAddress |= ReadMemory((regs.PB << 16) | regs.PC++) << 16;

	return 0;
}

uint8_t CPU::ABSLongIndexX() {
	fetchAddress = ReadMemory((regs.PB << 16) | regs.PC++);
	fetchAddress |= ReadMemory((regs.PB << 16) | regs.PC++) << 8;
	fetchAddress |= ReadMemory((regs.PB << 16) | regs.PC++) << 16;
	
	fetchAddress += regs.X.HL;

	return 0;
}

uint8_t CPU::ABSIndexX() {
	fetchAddress = ReadMemory((regs.PB << 16) | regs.PC++);
	fetchAddress |= ReadMemory((regs.PB << 16) | regs.PC++) << 8;
	
	uint8_t extraCycles = 0;
	if (!(regs.P & XFlag) || ((fetchAddress & 0xFF00) != ((fetchAddress + regs.X.HL) & 0xFF00))) {
		extraCycles++;
	}
	fetchAddress += regs.X.HL;

	return extraCycles;
}

uint8_t CPU::ABSIndexY() {
	fetchAddress = ReadMemory((regs.PB << 16) | regs.PC++);
	fetchAddress |= ReadMemory((regs.PB << 16) | regs.PC++) << 8;
	
	uint8_t extraCycles = 0;
	if (!(regs.P & XFlag) || ((fetchAddress & 0xFF00) != ((fetchAddress + regs.Y.HL) & 0xFF00))) {
		extraCycles++;
	}
	fetchAddress += regs.Y.HL;

	return extraCycles;
}

uint8_t CPU::DirectPage() {
	uint8_t directPageOffset = ReadMemory((regs.PB << 16) | regs.PC++);

	fetchAddress = regs.DP + directPageOffset;
	return (regs.DP & 0xFF) ? 1 : 0;
}

uint8_t CPU::DirectPageIndexedX() {
	uint8_t directPageOffset = ReadMemory((regs.PB << 16) | regs.PC++);

	fetchAddress = regs.DP + directPageOffset + regs.X.HL;
	return (regs.DP & 0xFF) ? 1 : 0;
}

uint8_t CPU::DirectPageIndexedY() {
	uint8_t directPageOffset = ReadMemory((regs.PB << 16) | regs.PC++);

	fetchAddress = regs.DP + directPageOffset + regs.Y.HL;
	return (regs.DP & 0xFF) ? 1 : 0;
}

uint8_t CPU::DirectPageIndirect() {
	uint8_t directPageOffset = ReadMemory((regs.PB << 16) | regs.PC++);

	uint16_t indirectAddress = regs.DP + directPageOffset;

	fetchAddress = ReadMemory(indirectAddress);
	fetchAddress |= ReadMemory((indirectAddress + 1) & 0xFFFF) << 8;
	return (regs.DP & 0xFF) ? 1 : 0;
}

uint8_t CPU::DirectPageIndirectX() {
	uint8_t directPageOffset = ReadMemory((regs.PB << 16) | regs.PC++);

	uint16_t indirectAddress = regs.DP + directPageOffset + regs.X.HL;

	fetchAddress = ReadMemory(indirectAddress);
	fetchAddress |= ReadMemory((indirectAddress + 1) & 0xFFFF) << 8;
	return (regs.DP & 0xFF) ? 1 : 0;
}

uint8_t CPU::DirectPageIndirectIndexedY() {
	uint8_t directPageOffset = ReadMemory((regs.PB << 16) | regs.PC++);
	
	uint16_t indirectAddress = regs.DP + directPageOffset;

	fetchAddress = ReadMemory(indirectAddress);
	fetchAddress |= ReadMemory((indirectAddress + 1) & 0xFFFF) << 8;

	uint8_t extraCycles = 0;
	if (!(regs.P & XFlag) || ((fetchAddress & 0xFF00) != ((fetchAddress + regs.Y.HL) & 0xFF00))) {
		extraCycles++;
	}
	fetchAddress += regs.Y.HL;

	return extraCycles + (regs.DP & 0xFF) ? 1 : 0;
}

uint8_t CPU::DirectPageIndirectLong() {
	uint8_t directPageOffset = ReadMemory((regs.PB << 16) | regs.PC++);

	uint16_t indirectAddress = regs.DP + directPageOffset;

	fetchAddress = ReadMemory(indirectAddress);
	fetchAddress |= ReadMemory((indirectAddress + 1) & 0xFFFF) << 8;
	fetchAddress |= ReadMemory((indirectAddress + 2) & 0xFFFF) << 16;
	return (regs.DP & 0xFF) ? 1 : 0;
}

uint8_t CPU::DirectPageIndirectLongIndexedY() {
	uint8_t directPageOffset = ReadMemory((regs.PB << 16) | regs.PC++);
	
	uint16_t indirectAddress = regs.DP + directPageOffset;

	fetchAddress = ReadMemory(indirectAddress);
	fetchAddress |= ReadMemory((indirectAddress + 1) & 0xFFFF) << 8;
	fetchAddress |= ReadMemory((indirectAddress + 2) & 0xFFFF) << 16;

	fetchAddress += regs.Y.HL;

	return (regs.DP & 0xFF) ? 1 : 0;
}

uint8_t CPU::StackRel() {
	fetchAddress = ReadMemory((regs.PB << 16) | regs.PC++);
	fetchAddress += regs.SP;
	fetchAddress &= 0xFFFF;

	return 0;
}

uint8_t CPU::StackRelativeIndirectIndexedY() {
	uint16_t indirectAddress = ReadMemory((regs.PB << 16) | regs.PC++);
	indirectAddress += regs.SP;
	indirectAddress &= 0xFFFF;

	fetchAddress = ReadMemory(indirectAddress);
	fetchAddress |= ReadMemory((indirectAddress + 1) & 0xFFFF) << 8;

	fetchAddress |= regs.DB << 16;

	fetchAddress += regs.Y.HL;

	fetchAddress &= 0xFFFFFF;

	return 0;
}

// Opcodes

uint8_t CPU::ADC() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & MFlag));
	
	uint8_t carry = (regs.P & CFlag) ? 1 : 0;
	if (regs.P & MFlag) {
		// Decimal Mode
		if (regs.P & DFlag) {
			uint16_t result = (regs.A.L & 0x0F) + (fetched & 0x0F) + carry;
			if (result > 0x09) {
				result += 0x06;
			}
			carry = (result > 0x0F) ? 1 : 0;

			result = (regs.A.L & 0xF0) + (fetched & 0xF0) + (carry << 4) + (result & 0x0F);
			SetFlag(VFlag, (int8_t)result < -128 || result > 127);
			if (result > 0x9F) {
				result += 0x60;
			}

			CheckNFlag8Bits(result);
			CheckZFlag8Bits(result);
			SetFlag(CFlag, (uint8_t)result < regs.A.L);

			regs.A.L = result;

		} else {
			uint8_t result = regs.A.L + fetched + carry;
			CheckNFlag8Bits(result);
			CheckZFlag8Bits(result);

			SetFlag(CFlag, result < regs.A.L);

			SetFlag(VFlag, (int8_t)result < -128 || result > 127);

			regs.A.L = result;
		}
	} else {
		// Decimal Mode
		if (regs.P & DFlag) {
			uint32_t result = (regs.A.HL & 0x000F) + (fetched & 0x000F) + carry;
			if (result > 0x0009) {
				result += 0x0006;
			}
			carry = (result > 0x000F) ? 1 : 0;

			result = (regs.A.HL & 0x00F0) + (fetched & 0x00F0) + (carry << 4) + (result & 0x000F);
			if (result > 0x009F) {
				result += 0x0060;
			}
			carry = (result > 0x00FF) ? 1 : 0;

			result = (regs.A.HL & 0x0F00) + (fetched & 0x0F00) + (carry << 8) + (result & 0x00FF);
			if (result > 0x09FF) {
				result += 0x0600;
			}
			carry = (result > 0x0FFF) ? 1 : 0;

			result = (regs.A.HL & 0xF000) + (fetched & 0xF000) + (carry << 12) + (result & 0x0FFF);
			SetFlag(VFlag, (int16_t)result < -32768 || result > 32767);
			if (result > 0x9FFF) {
				result += 0x6000;
			}

			CheckNFlag16Bits(result);
			CheckZFlag16Bits(result);
			SetFlag(CFlag, (uint16_t)result < regs.A.HL);

			regs.A.HL = result;

		} else {
			uint16_t result = regs.A.HL + fetched + carry;
			CheckNFlag16Bits(result);
			CheckZFlag16Bits(result);

			SetFlag(CFlag, result < regs.A.HL);

			SetFlag(VFlag, (int16_t)result < -32768 || result > 32767);

			regs.A.HL = result;
		}
	}

	return extraCycles;
}

uint8_t CPU::AND() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & MFlag));

	if (regs.P & MFlag) {
		regs.A.L = regs.A.L & fetched;
		CheckNFlag8Bits(regs.A.L);
		CheckZFlag8Bits(regs.A.L);
	} else {
		regs.A.HL = regs.A.HL & fetched;
		CheckNFlag16Bits(regs.A.HL);
		CheckZFlag16Bits(regs.A.HL);
	}
	return extraCycles;
}

uint8_t CPU::ASL() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & MFlag));
	
	if (regs.P & MFlag) {
		SetFlag(CFlag, fetched & 0x80);
		fetched = fetched << 1;
		SetFlag(NFlag, fetched & 0x80);
		SetFlag(ZFlag, !(fetched & 0xFF));
	} else {
		SetFlag(CFlag, fetched & 0x8000);
		fetched = fetched << 1;
		SetFlag(NFlag, fetched & 0x8000);
		SetFlag(ZFlag, !(fetched & 0xFFFF));
	}

	extraCycles += WriteValue(opTable[opcode].addMode, fetched, !(regs.P & MFlag), true);

	return extraCycles;
}

uint8_t CPU::BCC() {
	int8_t offset = ReadMemory((regs.PB << 16) | regs.PC++);
	
	if (!(regs.P & CFlag)) {
		regs.PC += offset;

		if (emulationMode) {
			assert(false && "Emulation Mode in BCC");
		}
		return 1;
	}

	return 0;
}

uint8_t CPU::BCS() {
	int8_t offset = ReadMemory((regs.PB << 16) | regs.PC++);
	
	if (regs.P & CFlag) {
		regs.PC += offset;

		if (emulationMode) {
			assert(false && "Emulation Mode in BCS");
		}
		return 1;
	}

	return 0;
}

uint8_t CPU::BEQ() {
	int8_t offset = ReadMemory((regs.PB << 16) | regs.PC++);
	
	if (regs.P & ZFlag) {
		regs.PC += offset;

		if (emulationMode) {
			assert(false && "Emulation Mode in BEQ");
		}
		return 1;
	}

	return 0;

}

uint8_t CPU::BIT() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & MFlag));
	uint16_t result = regs.A.HL & fetched;
	if (opTable[opcode].addMode != AddMode::Immediate) {
		if (regs.P & MFlag) {
			SetFlag(NFlag, fetched & 0x80);
			
			SetFlag(VFlag, fetched & 0x40);
		} else {
			SetFlag(NFlag, fetched & 0x8000);

			SetFlag(VFlag, fetched & 0x4000);
		}	
	}
	if (regs.P & MFlag) {
		CheckZFlag8Bits(result);
	} else {
		CheckZFlag16Bits(result);
	}
	return extraCycles;
}

uint8_t CPU::BMI() {
	int8_t offset = ReadMemory((regs.PB << 16) | regs.PC++);
	
	if (regs.P & NFlag) {
		regs.PC += offset;

		if (emulationMode) {
			assert(false && "Emulation Mode in BMI");
		}
		return 1;
	}

	return 0;
}

uint8_t CPU::BNE() {
	int8_t offset = ReadMemory((regs.PB << 16) | regs.PC++);
	
	if (!(regs.P & ZFlag)) {
		if (emulationMode) {
			if (((regs.PC + offset) & 0xFF00) != (regs.PC & 0xFF00)) {
				regs.PC += offset;
				return 2;
			}
		}
		regs.PC += offset;
		return 1;
	}

	return 0;
}

uint8_t CPU::BPL() {
	int8_t offset = ReadMemory((regs.PB << 16) | regs.PC++);
	
	if (!(regs.P & NFlag)) {
		regs.PC += offset;

		if (emulationMode) {
			assert(false && "Emulation Mode in BNE");
		}
		return 1;
	}

	return 0;
}

uint8_t CPU::BRA() {
	int8_t offset = ReadMemory((regs.PB << 16) | regs.PC++);
	regs.PC += offset;
	assert(!emulationMode);
	return 0;
}

uint8_t CPU::BRK() {
	if (!emulationMode) {
		PushStack(regs.PB);
	}
	regs.PC++;
	PushStack(regs.PC >> 8);
	PushStack(regs.PC & 0xFF);
	PushStack(regs.P);

	SetFlag(IFlag, true);
	SetFlag(DFlag, false);

	regs.PC = excVectors.BRKVector;

	return emulationMode ? 0 : 1;
}

uint8_t CPU::BRL() {
	int16_t offset = ReadMemory((regs.PB << 16) | regs.PC++);
	offset |= ReadMemory((regs.PB << 16) | regs.PC++) << 8;
	regs.PC += offset;
	assert(!emulationMode);
	return 0;
}

uint8_t CPU::BVC() {
	int8_t offset = ReadMemory((regs.PB << 16) | regs.PC++);
	
	if (!(regs.P & VFlag)) {
		regs.PC += offset;

		if (emulationMode) {
			assert(false && "Emulation Mode in BVC");
		}
		return 1;
	}

	return 0;
}

uint8_t CPU::BVS() {
	int8_t offset = ReadMemory((regs.PB << 16) | regs.PC++);
	
	if (regs.P & VFlag) {
		regs.PC += offset;

		if (emulationMode) {
			assert(false && "Emulation Mode in BVS");
		}
		return 1;
	}

	return 0;
}

uint8_t CPU::CLC() {
	SetFlag(CFlag, false);
	return 0;
}

uint8_t CPU::CLD() {
	SetFlag(DFlag, false);
	return 0;
}

uint8_t CPU::CLI() {
	SetFlag(IFlag, false);
	return 0;
}

uint8_t CPU::CLV() {
	SetFlag(VFlag, false);
	return 0;
}

uint8_t CPU::CMP() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & MFlag));

	if (regs.P & MFlag) {
		uint8_t result = regs.A.L - fetched;
		CheckNFlag8Bits(result);
		CheckZFlag8Bits(result);
		SetFlag(CFlag, regs.A.L >= fetched);
	} else {
		uint16_t result = regs.A.HL - fetched;
		CheckNFlag16Bits(result);
		CheckZFlag16Bits(result);
		SetFlag(CFlag, regs.A.HL >= fetched);
	}
	return extraCycles;
}

uint8_t CPU::COP() {
	if (!emulationMode) {
		PushStack(regs.PB);
	}
	regs.PC++;
	PushStack(regs.PC >> 8);
	PushStack(regs.PC & 0xFF);
	PushStack(regs.P);

	SetFlag(IFlag, true);
	SetFlag(DFlag, false);

	regs.PC = excVectors.COPVector;

	return emulationMode ? 0 : 1;
}

uint8_t CPU::CPX() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & XFlag));

	if (!(regs.P & XFlag)) {
		CheckNFlag16Bits(regs.X.HL - fetched);
		SetFlag(ZFlag, regs.X.HL == fetched);
		
		SetFlag(CFlag, regs.X.HL >= fetched);
	} else {
		CheckNFlag8Bits(regs.X.L - fetched);
		SetFlag(ZFlag, regs.X.L == fetched);

		SetFlag(CFlag, regs.X.L >= fetched);
	}

	return extraCycles;
}

uint8_t CPU::CPY() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & XFlag));

	if (!(regs.P & XFlag)) {
		CheckNFlag16Bits(regs.Y.HL - fetched);
		SetFlag(ZFlag, regs.Y.HL == fetched);
		
		SetFlag(CFlag, regs.Y.HL >= fetched);
	} else {
		CheckNFlag8Bits(regs.Y.L - fetched);
		SetFlag(ZFlag, regs.Y.L == fetched);

		SetFlag(CFlag, regs.Y.L >= fetched);
	}

	return extraCycles;
}

uint8_t CPU::DEC() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & MFlag));

	fetched--;

	if (regs.P & MFlag) {
		CheckNFlag8Bits(fetched);
		CheckZFlag8Bits(fetched);
	} else {
		CheckNFlag16Bits(fetched);
		CheckZFlag16Bits(fetched);
	}

	extraCycles += WriteValue(opTable[opcode].addMode, fetched, !(regs.P & MFlag), true);

	return extraCycles;
}

uint8_t CPU::DEX() {
	if (!(regs.P & XFlag)) {
		regs.X.HL--;
		CheckNFlag16Bits(regs.X.HL);
		CheckZFlag16Bits(regs.X.HL);
	} else {
		regs.X.L--;
		CheckNFlag8Bits(regs.X.L);
		CheckZFlag8Bits(regs.X.L);
	}
	return 0;
}

uint8_t CPU::DEY() {
	if (!(regs.P & XFlag)) {
		regs.Y.HL--;
		CheckNFlag16Bits(regs.Y.HL);
		CheckZFlag16Bits(regs.Y.HL);
	} else {
		regs.Y.L--;
		CheckNFlag8Bits(regs.Y.L);
		CheckZFlag8Bits(regs.Y.L);
	}
	return 0;
}

uint8_t CPU::EOR() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & MFlag));

	if (regs.P & MFlag) {
		regs.A.L = regs.A.L ^ fetched;
		CheckNFlag8Bits(regs.A.L);
		CheckZFlag8Bits(regs.A.L);
	} else {
		regs.A.HL = regs.A.HL ^ fetched;
		CheckNFlag16Bits(regs.A.HL);
		CheckZFlag16Bits(regs.A.HL);
	}
	return extraCycles;

}

uint8_t CPU::INC() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & MFlag));

	fetched++;

	if (regs.P & MFlag) {
		CheckNFlag8Bits(fetched);
		CheckZFlag8Bits(fetched);
	} else {
		CheckNFlag16Bits(fetched);
		CheckZFlag16Bits(fetched);
	}

	extraCycles += WriteValue(opTable[opcode].addMode, fetched, !(regs.P & MFlag), true);

	return extraCycles;
}

uint8_t CPU::INX() {
	if (regs.P & XFlag) {
		regs.X.L++;
		CheckNFlag8Bits(regs.X.L);
		CheckZFlag8Bits(regs.X.L);
	} else {
		regs.X.HL++;
		CheckNFlag16Bits(regs.X.HL);
		CheckZFlag16Bits(regs.X.HL);
	}
	return 0;
}

uint8_t CPU::INY() {
	if (regs.P & XFlag) {
		regs.Y.L++;
		CheckNFlag8Bits(regs.Y.L);
		CheckZFlag8Bits(regs.Y.L);
	} else {
		regs.Y.HL++;
		CheckNFlag16Bits(regs.Y.HL);
		CheckZFlag16Bits(regs.Y.HL);
	}
	return 0;
}

uint8_t CPU::JML() {
	switch (opTable[opcode].addMode) {
		case AddMode::AbsoluteLong:
		{
			ABSLong();
			break;
		}

		case AddMode::AbsoluteIndirectLong:
		{
			uint16_t indirectAddres = ReadMemory(regs.PC++);
			indirectAddres |= ReadMemory(regs.PC++) << 8;
			
			fetchAddress = ReadMemory(indirectAddres);
			fetchAddress |= ReadMemory((indirectAddres + 1) & 0xFFFF) << 8;

			fetchAddress |= ReadMemory((indirectAddres + 2) & 0xFFFF) << 16;

			break;
		}

		default:
			assert(false && "JML Addressing Mode not handled.");
			break;
	}

	regs.PB = fetchAddress >> 16;

	regs.PC = fetchAddress & 0xFFFF;

	return 0;
}

uint8_t CPU::JMP() {
	switch (opTable[opcode].addMode) {
		case AddMode::Absolute:
		{
			ABS();
			break;
		}

		case AddMode::AbsoluteIndirect:
		{
			uint16_t indirectAddres = ReadMemory((regs.PB << 16) | regs.PC++);
			indirectAddres |= ReadMemory((regs.PB << 16) | regs.PC++) << 8;
			
			fetchAddress = ReadMemory(indirectAddres);
			fetchAddress |= ReadMemory((indirectAddres + 1) & 0xFFFF) << 8;

			break;
		}

		case AddMode::AbsoluteIndexedIndirectX:
		{
			uint16_t indirectAddres = ReadMemory((regs.PB << 16) | regs.PC++);
			indirectAddres |= ReadMemory((regs.PB << 16) | regs.PC++) << 8;
			
			fetchAddress = ReadMemory((regs.PB << 16) | ((indirectAddres + regs.X.HL) & 0xFFFF));
			fetchAddress |= ReadMemory((regs.PB << 16) | ((indirectAddres + regs.X.HL + 1) & 0xFFFF)) << 8;

			break;
		}

		default:
			assert(false && "JMP Addressig Mode not handled.");
			break;
	}

	regs.PC = fetchAddress;
	return 0;
}

uint8_t CPU::JSL() {
	ABSLong();

	PushStack(regs.PB);
	regs.PC--;
	PushStack(regs.PC >> 8);
	PushStack(regs.PC);

	regs.PB = fetchAddress >> 16;
	regs.PC = fetchAddress & 0xFFFF;

	return 0;
}

uint8_t CPU::JSR() {
	if (opTable[opcode].addMode == AddMode::Absolute) {
		ABS();
	}
	else {
		uint16_t indirectAddres = ReadMemory((regs.PB << 16) | regs.PC++);
		indirectAddres |= ReadMemory((regs.PB << 16) | regs.PC++) << 8;
			
		fetchAddress = ReadMemory((regs.PB << 16) | ((indirectAddres + regs.X.HL) & 0xFFFF));
		fetchAddress |= ReadMemory((regs.PB << 16) | ((indirectAddres + regs.X.HL + 1) & 0xFFFF)) << 8;
	}

	regs.PC--;
	PushStack(regs.PC >> 8);
	PushStack(regs.PC);

	regs.PC = fetchAddress;

	return 0;
}

uint8_t CPU::LDA() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & MFlag));

	if (regs.P & MFlag) {
		regs.A.L = fetched;
		CheckNFlag8Bits(regs.A.L);
		CheckZFlag8Bits(regs.A.L);
	} else {
		regs.A.HL = fetched;
		CheckNFlag16Bits(regs.A.HL);
		CheckZFlag16Bits(regs.A.HL);
	}
	return extraCycles;
}

uint8_t CPU::LDX() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & XFlag));
	
	if (regs.P & XFlag) {
		regs.X.L = fetched;
		CheckNFlag8Bits(regs.X.L);
		CheckZFlag8Bits(regs.X.L);
	} else {
		regs.X.HL = fetched;
		CheckNFlag16Bits(regs.X.HL);
		CheckZFlag16Bits(regs.X.HL);
	}
	return extraCycles;
}

uint8_t CPU::LDY() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & XFlag));
	
	if (regs.P & XFlag) {
		regs.Y.L = fetched;
		CheckNFlag8Bits(regs.Y.L);
		CheckZFlag8Bits(regs.Y.L);
	} else {
		regs.Y.HL = fetched;
		CheckNFlag16Bits(regs.Y.HL);
		CheckZFlag16Bits(regs.Y.HL);
	}
	return extraCycles;
}

uint8_t CPU::LSR() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & MFlag));
	
	SetFlag(NFlag, false);
	SetFlag(CFlag, fetched & 0x01);
	fetched = fetched >> 1;
	if (regs.P & MFlag) {
		SetFlag(ZFlag, !(fetched & 0xFF));
	} else {
		SetFlag(ZFlag, !(fetched & 0xFFFF));
	}

	extraCycles += WriteValue(opTable[opcode].addMode, fetched, !(regs.P & MFlag), true);

	return extraCycles;
}

uint8_t CPU::MVN() {
	if (!movActive) {
		srcBank = ReadMemory((regs.PB << 16) | regs.PC++);
		regs.DB = ReadMemory((regs.PB << 16) | regs.PC++);
		regs.PC -= 2;
		movActive = true;
	}
	assert(!(regs.P & XFlag) && "MVN XFlag");
	uint8_t value = ReadMemory((srcBank << 16) | regs.X.HL);
	WriteMemory((regs.DB << 16) | regs.Y.HL, value);

	regs.A.HL--;
	regs.X.HL++;
	regs.Y.HL++;

	if (regs.A.HL == 0xFFFF) {
		movActive = false;
		regs.PC += 2;
	} else {
		regs.PC--;
	}

	return 7;
}

uint8_t CPU::MVP() {
	if (!movActive) {
		srcBank = ReadMemory((regs.PB << 16) | regs.PC++);
		regs.DB = ReadMemory((regs.PB << 16) | regs.PC++);
		regs.PC -= 2;
		movActive = true;
	}
	assert(!(regs.P & XFlag) && "MVN XFlag");
	uint8_t value = ReadMemory((srcBank << 16) | regs.X.HL);
	WriteMemory((regs.DB << 16) | regs.Y.HL, value);

	regs.A.HL--;
	regs.X.HL--;
	regs.Y.HL--;

	if (regs.A.HL == 0xFFFF) {
		movActive = false;
		regs.PC += 2;
	} else {
		regs.PC--;
	}

	return 7;
}

uint8_t CPU::NOP() {
	return 0;
}

uint8_t CPU::ORA() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & MFlag));

	if (regs.P & MFlag) {
		regs.A.L |= fetched;

		CheckNFlag8Bits(regs.A.L);
		CheckZFlag8Bits(regs.A.L);
	} else {
		regs.A.HL |= fetched;

		CheckNFlag16Bits(regs.A.HL);
		CheckZFlag16Bits(regs.A.HL);
	}

	return extraCycles;
}

uint8_t CPU::PEA() {
	ABS();
	PushStack(fetchAddress >> 8);
	PushStack(fetchAddress);
	return 0;
}

uint8_t CPU::PEI() {
	uint8_t extraCycles = DirectPageIndirect();
	PushStack(fetchAddress >> 8);
	PushStack(fetchAddress);
	return extraCycles;
}

uint8_t CPU::PER() {
	ABS();
	uint16_t value = regs.PC + fetchAddress;
	PushStack(value >> 8);
	PushStack(value);
	return 0;
}

uint8_t CPU::PHA() {
	uint8_t extraCycles = 0;
	if (!(regs.P & MFlag)) {
		extraCycles++;
		PushStack(regs.A.HL >> 8);
	}
	PushStack(regs.A.L);

	return extraCycles;
}

uint8_t CPU::PHB() {
	PushStack(regs.DB);
	return 0;
}

uint8_t CPU::PHD() {
	PushStack(regs.DP >> 8);
	PushStack(regs.DP);
	return 0;
}

uint8_t CPU::PHK() {
	PushStack(regs.PB);
	return 0;
}

uint8_t CPU::PHP() {
	PushStack(regs.P);
	return 0;
}

uint8_t CPU::PHX() {
	uint8_t extraCycles = 0;
	if (!(regs.P & XFlag)) {
		extraCycles++;
		PushStack(regs.X.HL >> 8);
	}
	PushStack(regs.X.L);

	return extraCycles;
}

uint8_t CPU::PHY() {
	uint8_t extraCycles = 0;
	if (!(regs.P & XFlag)) {
		extraCycles++;
		PushStack(regs.Y.HL >> 8);
	}
	PushStack(regs.Y.L);

	return extraCycles;
}

uint8_t CPU::PLA() {
	regs.A.L = PullStack();
	if (!(regs.P & MFlag)) {
		regs.A.HL &= 0xFF;
		regs.A.HL |= PullStack() << 8;
		CheckNFlag16Bits(regs.A.HL);
		CheckZFlag16Bits(regs.A.HL);
		return 1;
	}
	CheckNFlag8Bits(regs.A.L);
	CheckZFlag8Bits(regs.A.L);
	return 0;
}

uint8_t CPU::PLB() {
	regs.DB = PullStack();
	CheckZFlag8Bits(regs.DB);
	CheckNFlag8Bits(regs.DB);
	return 0;
}

uint8_t CPU::PLD() {
	regs.DP = PullStack();
	regs.DP |= PullStack() << 8;
	CheckNFlag16Bits(regs.DP);
	CheckZFlag16Bits(regs.DP);
	return 0;
}

uint8_t CPU::PLP() {
	regs.P = PullStack();
	if (emulationMode) {
		SetFlag(MFlag, true);
		SetFlag(XFlag, true);
	}
	if (regs.P & XFlag) {
		regs.X.HL &= 0xFF;
		regs.Y.HL &= 0xFF;
	}
	return 0;
}

uint8_t CPU::PLX() {
	regs.X.L = PullStack();
	if (!(regs.P & XFlag)) {
		regs.X.HL &= 0xFF;
		regs.X.HL |= PullStack() << 8;
		CheckNFlag16Bits(regs.X.HL);
		CheckZFlag16Bits(regs.X.HL);
		return 1;
	}
	CheckZFlag8Bits(regs.X.L);
	CheckNFlag8Bits(regs.X.L);
	return 0;
}

uint8_t CPU::PLY() {
	regs.Y.L = PullStack();
	if (!(regs.P & XFlag)) {
		regs.Y.HL &= 0xFF;
		regs.Y.HL |= PullStack() << 8;
		CheckNFlag16Bits(regs.Y.HL);
		CheckZFlag16Bits(regs.Y.HL);
		return 1;
	}
	CheckZFlag8Bits(regs.Y.L);
	CheckNFlag8Bits(regs.Y.L);
	return 0;
}

uint8_t CPU::REP() {
	FetchValue(opTable[opcode].addMode, false);
	regs.P &= ~fetched;

	return 0;
}

uint8_t CPU::ROL() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & MFlag));

	if (regs.P & MFlag) {
		bool hasCarry = fetched & 0x80;
		if (regs.P & CFlag) {
			fetched = (fetched << 1) + 1;
		} else {
			fetched = fetched << 1;
		}
		fetched &= 0xFF;

		SetFlag(CFlag, hasCarry);
		CheckNFlag8Bits(fetched);
		CheckZFlag8Bits(fetched);
	} else {
		bool hasCarry = fetched & 0x8000;
		if (regs.P & CFlag) {
			fetched = (fetched << 1) + 1;
		} else {
			fetched = fetched << 1;
		}
		fetched &= 0xFFFF;

		SetFlag(CFlag, hasCarry);
		CheckNFlag16Bits(fetched);
		CheckZFlag16Bits(fetched);
	}

	extraCycles += WriteValue(opTable[opcode].addMode, fetched, !(regs.P & MFlag), true);

	return extraCycles;
}

uint8_t CPU::ROR() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & MFlag));
	bool hasCarry = fetched & 0x01;
	if (regs.P & MFlag) {
		if (regs.P & CFlag) {
			fetched = (fetched >> 1) | 0x80;
		} else {
			fetched = fetched >> 1;
		}
		fetched &= 0xFF;

		CheckNFlag8Bits(fetched);
		CheckZFlag8Bits(fetched);
	} else {
		if (regs.P & CFlag) {
			fetched = (fetched >> 1) | 0x8000;
		} else {
			fetched = fetched >> 1;
		}
		fetched &= 0xFFFF;

		CheckNFlag16Bits(fetched);
		CheckZFlag16Bits(fetched);
	}

	SetFlag(CFlag, hasCarry);
	extraCycles += WriteValue(opTable[opcode].addMode, fetched, !(regs.P & MFlag), true);

	return extraCycles;
}

uint8_t CPU::RTI() {
	regs.P = PullStack();
	regs.PC = PullStack();
	regs.PC |= PullStack() << 8;

	if (!emulationMode) {
		regs.PB = PullStack();
	}

	return emulationMode ? 0 : 1;
}

uint8_t CPU::RTL() {
	regs.PC = PullStack();
	regs.PC |= PullStack() << 8;
	regs.PC++;
	regs.PB = PullStack();

	return 0;
}

uint8_t CPU::RTS() {
	regs.PC = PullStack();
	regs.PC |= PullStack() << 8;
	regs.PC++;

	return 0;
}

uint8_t CPU::SBC() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & MFlag));
	uint8_t carry = (regs.P & CFlag) ? 0 : 1;
	if (regs.P & MFlag) {
		// Decimal Mode
		if (regs.P & DFlag) {
			uint16_t result = (regs.A.L & 0x0F) - (fetched & 0x0F) - carry;
			if (result > 0x09) {
				result -= 0x06;
			}
			carry = (result > 0x0F) ? 1 : 0;

			result = (regs.A.L & 0xF0) - (fetched & 0xF0) - (carry << 4) + (result & 0x0F);
			SetFlag(VFlag, (int16_t)result < -128 || (int16_t)result > 127);
			if (result > 0x9F) {
				result -= 0x60;
			}

			CheckNFlag8Bits(result);
			CheckZFlag8Bits(result);
			SetFlag(CFlag, (uint8_t)result < regs.A.L);

			regs.A.L = result;

		} else {
			uint8_t result = regs.A.L - fetched - carry;
			CheckNFlag8Bits(result);
			CheckZFlag8Bits(result);

			SetFlag(CFlag, result < regs.A.L);

			SetFlag(VFlag, (int8_t)result < -128 || result > 127);

			regs.A.L = result;
		}
	} else {
		// Decimal Mode
		if (regs.P & DFlag) {
			uint32_t result = (regs.A.HL & 0x000F) - (fetched & 0x000F) - carry;
			if (result > 0x0009) {
				result -= 0x0006;
			}
			carry = (result > 0x000F) ? 1 : 0;

			result = (regs.A.HL & 0x00F0) - (fetched & 0x00F0) - (carry << 4) + (result & 0x000F);
			if (result > 0x009F) {
				result -= 0x0060;
			}
			carry = (result > 0x00FF) ? 1 : 0;

			result = (regs.A.HL & 0x0F00) - (fetched & 0x0F00) - (carry << 8) + (result & 0x00FF);
			if (result > 0x09FF) {
				result -= 0x0600;
			}
			carry = (result > 0x0FFF) ? 1 : 0;

			result = (regs.A.HL & 0xF000) - (fetched & 0xF000) - (carry << 12) + (result & 0x0FFF);
			SetFlag(VFlag, (int32_t)result < -32768 || (int32_t)result > 32767);
			if (result > 0x9FFF) {
				result -= 0x6000;
			}

			CheckNFlag16Bits(result);
			CheckZFlag16Bits(result);
			SetFlag(CFlag, (uint16_t)result < regs.A.HL);

			regs.A.HL = result;

		} else {
			uint16_t result = regs.A.HL - fetched - carry;
			CheckNFlag16Bits(result);
			CheckZFlag16Bits(result);

			SetFlag(CFlag, result < regs.A.HL);

			SetFlag(VFlag, (int16_t)result < -32768 || result > 32767);

			regs.A.HL = result;
		}
	}

	return extraCycles;

}

uint8_t CPU::SEC() {
	SetFlag(CFlag, true);
	return 0;
}

uint8_t CPU::SED() {
	SetFlag(DFlag, true);
	return 0;
}

uint8_t CPU::SEI() {
	SetFlag(IFlag, true);
	return 0;
}

uint8_t CPU::SEP() {
	FetchValue(opTable[opcode].addMode, false);
	regs.P |= fetched;

	if (regs.P & XFlag) {
		regs.X.HL &= 0xFF;
		regs.Y.HL &= 0xFF;
	}

	return 0;
}

uint8_t CPU::STA() {
	uint16_t value;
	if (regs.P & MFlag) {
		value = regs.A.L;
	} else {
		value = regs.A.HL;
	}
	uint8_t extraCycles = WriteValue(opTable[opcode].addMode, value, !(regs.P & MFlag));

	return extraCycles;
}

uint8_t CPU::STP() {
	assert(false && "STP OPCODE");
	return 0;
}

uint8_t CPU::STX() {
	uint16_t value;
	if (regs.P & XFlag) {
		value = regs.X.L;
	} else {
		value = regs.X.HL;
	}
	uint8_t extraCycles = WriteValue(opTable[opcode].addMode, value, !(regs.P & XFlag));

	return extraCycles;
}

uint8_t CPU::STY() {
	uint16_t value;
	if (regs.P & XFlag) {
		value = regs.Y.L;
	} else {
		value = regs.Y.HL;
	}
	uint8_t extraCycles = WriteValue(opTable[opcode].addMode, value, !(regs.P & XFlag));

	return extraCycles;
}

uint8_t CPU::STZ() {
	uint8_t extraCycles = WriteValue(opTable[opcode].addMode, 0, !(regs.P & MFlag));
	return extraCycles;
}

uint8_t CPU::TAX() {
	if (regs.P & XFlag) {
		regs.X.L = regs.A.L;
		CheckNFlag8Bits(regs.X.L);
		CheckZFlag8Bits(regs.X.L);
	} else {
		regs.X.HL = regs.A.HL;
		CheckNFlag16Bits(regs.X.HL);
		CheckZFlag16Bits(regs.X.HL);
	}

	return 0;
}

uint8_t CPU::TAY() {
	if (regs.P & XFlag) {
		regs.Y.L = regs.A.L;
		CheckNFlag8Bits(regs.Y.L);
		CheckZFlag8Bits(regs.Y.L);
	} else {
		regs.Y.HL = regs.A.HL;
		CheckNFlag16Bits(regs.Y.HL);
		CheckZFlag16Bits(regs.Y.HL);
	}

	return 0;
}

uint8_t CPU::TCD() {
	regs.DP = regs.A.HL;

	CheckNFlag16Bits(regs.DP);
	CheckZFlag16Bits(regs.DP);

	return 0;
}

uint8_t CPU::TCS() {
	regs.SP = regs.A.HL;

	return 0;
}

uint8_t CPU::TDC() {
	regs.A.HL = regs.DP;

	CheckNFlag16Bits(regs.A.HL);
	CheckZFlag16Bits(regs.A.HL);

	return 0;
}

uint8_t CPU::TRB() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & MFlag));

	uint16_t result = fetched & ~regs.A.HL;

	if (regs.P & MFlag) {
		CheckZFlag8Bits(fetched & regs.A.L);
	} else {
		CheckZFlag16Bits(fetched & regs.A.HL);
	}

	extraCycles += WriteValue(opTable[opcode].addMode, result, !(regs.P & MFlag), true);

	return extraCycles;
}

uint8_t CPU::TSB() {
	uint8_t extraCycles = FetchValue(opTable[opcode].addMode, !(regs.P & MFlag));

	uint16_t result = fetched | regs.A.HL;

	if (regs.P & MFlag) {
		CheckZFlag8Bits(fetched & regs.A.L);
	} else {
		CheckZFlag16Bits(fetched & regs.A.HL);
	}

	extraCycles += WriteValue(opTable[opcode].addMode, result, !(regs.P & MFlag), true);

	return extraCycles;
}

uint8_t CPU::TSC() {
	regs.A.HL = regs.SP;

	CheckNFlag16Bits(regs.A.HL);
	CheckZFlag16Bits(regs.A.HL);

	return 0;
}

uint8_t CPU::TSX() {
	if (regs.P & XFlag) {
		regs.X.HL = regs.SP & 0xFF;
		CheckNFlag8Bits(regs.X.L);
		CheckZFlag8Bits(regs.X.L);
	} else {
		regs.X.HL = regs.SP;
		CheckNFlag16Bits(regs.X.HL);
		CheckZFlag16Bits(regs.X.HL);
	}
	return 0;
}

uint8_t CPU::TXA() {
	if (regs.P & MFlag) {
		regs.A.L = regs.X.L;
		CheckNFlag8Bits(regs.A.L);
		CheckZFlag8Bits(regs.A.L);
	} else {
		if (regs.P & XFlag) {
			regs.A.HL = regs.X.HL & 0xFF;
		} else {
			regs.A.HL = regs.X.HL;
		}
		CheckNFlag16Bits(regs.A.HL);
		CheckZFlag16Bits(regs.A.HL);
	}
	return 0;
}

uint8_t CPU::TXS() {
	regs.SP = regs.X.HL;
	if (emulationMode) {
		regs.SP &= 0xFF;
		regs.SP |= 0x01FF;
	}
	return 0;
}

uint8_t CPU::TXY() {
	if (regs.P & XFlag) {
		regs.Y.L = regs.X.L;
		CheckNFlag8Bits(regs.Y.L);
		CheckZFlag8Bits(regs.Y.L);
	} else {
		regs.Y.HL = regs.X.HL;
		CheckNFlag16Bits(regs.Y.HL);
		CheckZFlag16Bits(regs.Y.HL);
	}
	return 0;
}

uint8_t CPU::TYA() {
	if (regs.P & MFlag) {
		regs.A.L = regs.Y.L;
		CheckNFlag8Bits(regs.A.L);
		CheckZFlag8Bits(regs.A.L);
	} else {
		if (regs.P & XFlag) {
			regs.A.HL = regs.Y.HL & 0xFF;
		} else {
			regs.A.HL = regs.Y.HL;
		}
		CheckNFlag16Bits(regs.A.HL);
		CheckZFlag16Bits(regs.A.HL);
	}
	return 0;
}

uint8_t CPU::TYX() {
	if (regs.P & XFlag) {
		regs.X.L = regs.Y.L;
		CheckNFlag8Bits(regs.X.L);
		CheckZFlag8Bits(regs.X.L);
	} else {
		regs.X.HL = regs.Y.HL;
		CheckNFlag16Bits(regs.X.HL);
		CheckZFlag16Bits(regs.X.HL);
	}
	return 0;
}

uint8_t CPU::WAI() {
	assert(false && "WAI OPCODE");
	return 0;
}

uint8_t CPU::WDM() {
	IMM(false);
	return 0;
}

uint8_t CPU::XBA() {
	uint8_t temp = regs.A.L;
	regs.A.L = regs.A.HL >> 8;
	regs.A.HL &= 0xFF;
	regs.A.HL |= temp << 8;
	CheckNFlag8Bits(regs.A.L);
	CheckZFlag8Bits(regs.A.L);

	return 0;
}

uint8_t CPU::XCE() {
	if (emulationMode) {
		SetFlag(CFlag, true);
		emulationMode = false;
	}
	else {
		emulationMode = (regs.P & CFlag);
		SetFlag(CFlag, false);
		if (emulationMode) {
			SetFlag(MFlag, true);
			SetFlag(XFlag, true);
			regs.SP &= 0xFF;
			regs.SP |= 0x100;
			regs.X.HL &= 0xFF;
			regs.Y.HL &= 0xFF;
		}
	}
	return 0;
}
