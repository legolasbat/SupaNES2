#include "CPU.h"
#include "SupaNES.h"

#include <cassert>
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

	nmi = 2;
	NMITIMEN = 0;

	joypad1Low = 0;
	joypad1High = 0;

	opTable[0x00] = {"brk", &CPU::BRK, AddMode::Implied, 7};
	opTable[0x01] = {"ora", &CPU::ORA, AddMode::DirectPageIndirectX, 6};
	opTable[0x02] = {"cop", &CPU::COP, AddMode::Implied, 7};
	opTable[0x03] = {"ora", &CPU::ORA, AddMode::StackRelative, 4};
	opTable[0x04] = {"tsb", &CPU::TSB, AddMode::DirectPage, 5};
	opTable[0x05] = {"ora", &CPU::ORA, AddMode::DirectPage, 3};
	opTable[0x06] = {"asl", &CPU::ASL, AddMode::DirectPage, 5};
	opTable[0x07] = {"ora", &CPU::ORA, AddMode::DirectPageIndirectLong, 6};
	opTable[0x08] = {"php", &CPU::PHP, AddMode::Implied, 3};
	opTable[0x09] = {"ora", &CPU::ORA, AddMode::Immediate, 2};
	opTable[0x0A] = {"asl", &CPU::ASL, AddMode::Implied, 2};
	opTable[0x0B] = {"phd", &CPU::PHD, AddMode::Implied, 4};
	opTable[0x0C] = {"tsb", &CPU::TSB, AddMode::Absolute, 6};
	opTable[0x0D] = {"ora", &CPU::ORA, AddMode::Absolute, 4};
	opTable[0x0E] = {"asl", &CPU::ASL, AddMode::Absolute, 6};
	opTable[0x0F] = {"ora", &CPU::ORA, AddMode::AbsoluteLong, 5};

	opTable[0x10] = {"bpl", &CPU::BPL, AddMode::Implied, 2};
	opTable[0x11] = {"ora", &CPU::ORA, AddMode::DirectPageIndirectIndexedY, 5};
	opTable[0x12] = {"ora", &CPU::ORA, AddMode::DirectPageIndirect, 5};
	opTable[0x13] = {"ora", &CPU::ORA, AddMode::StackRelativeIndirectIndexedY, 7};
	opTable[0x14] = {"trb", &CPU::TRB, AddMode::DirectPage, 5};
	opTable[0x15] = {"ora", &CPU::ORA, AddMode::DirectPageIndexedX, 4};
	opTable[0x16] = {"asl", &CPU::ASL, AddMode::DirectPageIndexedX, 6};
	opTable[0x17] = {"ora", &CPU::ORA, AddMode::DirectPageIndirectLongIndexedY, 6};
	opTable[0x18] = {"clc", &CPU::CLC, AddMode::Implied, 2};
	opTable[0x19] = {"ora", &CPU::ORA, AddMode::AbsoluteIndexedY, 4};
	opTable[0x1A] = {"inc", &CPU::INC, AddMode::Implied, 2};
	opTable[0x1B] = {"tcs", &CPU::TCS, AddMode::Implied, 2};
	opTable[0x1C] = {"trb", &CPU::TRB, AddMode::Absolute, 6};
	opTable[0x1D] = {"ora", &CPU::ORA, AddMode::AbsoluteIndexedX, 4};
	opTable[0x1E] = {"asl", &CPU::ASL, AddMode::AbsoluteIndexedX, 7};
	opTable[0x1F] = {"ora", &CPU::ORA, AddMode::AbsoluteLongIndexedX, 5};

	opTable[0x20] = {"jsr", &CPU::JSR, AddMode::Absolute, 6};
	opTable[0x21] = {"and", &CPU::AND, AddMode::DirectPageIndirectX, 6};
	opTable[0x22] = {"jsl", &CPU::JSL, AddMode::AbsoluteLong, 8};
	opTable[0x23] = {"and", &CPU::AND, AddMode::StackRelative, 4};
	opTable[0x24] = {"bit", &CPU::BIT, AddMode::DirectPage, 3};
	opTable[0x25] = {"and", &CPU::AND, AddMode::DirectPage, 3};
	opTable[0x26] = {"rol", &CPU::ROL, AddMode::DirectPage, 5};
	opTable[0x27] = {"and", &CPU::AND, AddMode::DirectPageIndirectLong, 6};
	opTable[0x28] = {"plp", &CPU::PLP, AddMode::Implied, 4};
	opTable[0x29] = {"and", &CPU::AND, AddMode::Immediate, 2};
	opTable[0x2A] = {"rol", &CPU::ROL, AddMode::Implied, 2};
	opTable[0x2B] = {"pld", &CPU::PLD, AddMode::Implied, 5};
	opTable[0x2C] = {"bit", &CPU::BIT, AddMode::Absolute, 4};
	opTable[0x2D] = {"and", &CPU::AND, AddMode::Absolute, 4};
	opTable[0x2E] = {"rol", &CPU::ROL, AddMode::Absolute, 6};
	opTable[0x2F] = {"and", &CPU::AND, AddMode::AbsoluteLong, 5};

	opTable[0x30] = {"bmi", &CPU::BMI, AddMode::Implied, 2};
	opTable[0x31] = {"and", &CPU::AND, AddMode::DirectPageIndirectIndexedY, 5};
	opTable[0x32] = {"and", &CPU::AND, AddMode::DirectPageIndirect, 5};
	opTable[0x33] = {"and", &CPU::AND, AddMode::StackRelativeIndirectIndexedY, 7};
	opTable[0x34] = {"bit", &CPU::BIT, AddMode::DirectPageIndexedX, 4};
	opTable[0x35] = {"and", &CPU::AND, AddMode::DirectPageIndexedX, 4};
	opTable[0x36] = {"rol", &CPU::ROL, AddMode::DirectPageIndexedX, 6};
	opTable[0x37] = {"and", &CPU::AND, AddMode::DirectPageIndirectLongIndexedY, 6};
	opTable[0x38] = {"sec", &CPU::SEC, AddMode::Implied, 2};
	opTable[0x39] = {"and", &CPU::AND, AddMode::AbsoluteIndexedY, 4};
	opTable[0x3A] = {"dec", &CPU::DEC, AddMode::Implied, 2};
	opTable[0x3B] = {"tsc", &CPU::TSC, AddMode::Implied, 2};
	opTable[0x3C] = {"bit", &CPU::BIT, AddMode::AbsoluteIndexedX, 4};
	opTable[0x3D] = {"and", &CPU::AND, AddMode::AbsoluteIndexedX, 4};
	opTable[0x3E] = {"rol", &CPU::ROL, AddMode::AbsoluteIndexedX, 7};
	opTable[0x3F] = {"and", &CPU::AND, AddMode::AbsoluteLongIndexedX, 5};

	opTable[0x40] = {"rti", &CPU::RTI, AddMode::Implied, 6};
	opTable[0x41] = {"eor", &CPU::EOR, AddMode::DirectPageIndirectX, 6};
	opTable[0x42] = {"wdm", &CPU::WDM, AddMode::Immediate, 2};	// Unused
	opTable[0x43] = {"eor", &CPU::EOR, AddMode::StackRelative, 4};
	opTable[0x44] = {"mvp", &CPU::MVP, AddMode::Implied, 0};
	opTable[0x45] = {"eor", &CPU::EOR, AddMode::DirectPage, 3};
	opTable[0x46] = {"lsr", &CPU::LSR, AddMode::DirectPage, 5};
	opTable[0x47] = {"eor", &CPU::EOR, AddMode::DirectPageIndirectLong, 6};
	opTable[0x48] = {"pha", &CPU::PHA, AddMode::Implied, 3};
	opTable[0x49] = {"eor", &CPU::EOR, AddMode::Immediate, 2};
	opTable[0x4A] = {"lsr", &CPU::LSR, AddMode::Implied, 2};
	opTable[0x4B] = {"phk", &CPU::PHK, AddMode::Implied, 3};
	opTable[0x4C] = {"jmp", &CPU::JMP, AddMode::Absolute, 3};
	opTable[0x4D] = {"eor", &CPU::EOR, AddMode::Absolute, 4};
	opTable[0x4E] = {"lsr", &CPU::LSR, AddMode::Absolute, 6};
	opTable[0x4F] = {"eor", &CPU::EOR, AddMode::AbsoluteLong, 5};

	opTable[0x50] = {"bvc", &CPU::BVC, AddMode::Implied, 2};
	opTable[0x51] = {"eor", &CPU::EOR, AddMode::DirectPageIndirectIndexedY, 5};
	opTable[0x52] = {"eor", &CPU::EOR, AddMode::DirectPageIndirect, 5};
	opTable[0x53] = {"eor", &CPU::EOR, AddMode::StackRelativeIndirectIndexedY, 7};
	opTable[0x54] = {"mvn", &CPU::MVN, AddMode::Implied, 0};
	opTable[0x55] = {"eor", &CPU::EOR, AddMode::DirectPageIndexedX, 4};
	opTable[0x56] = {"lsr", &CPU::LSR, AddMode::DirectPageIndexedX, 6};
	opTable[0x57] = {"eor", &CPU::EOR, AddMode::DirectPageIndirectLongIndexedY, 6};
	opTable[0x58] = {"cli", &CPU::CLI, AddMode::Implied, 2};
	opTable[0x59] = {"eor", &CPU::EOR, AddMode::AbsoluteIndexedY, 4};
	opTable[0x5A] = {"phy", &CPU::PHY, AddMode::Implied, 3};
	opTable[0x5B] = {"tcd", &CPU::TCD, AddMode::Implied, 2};
	opTable[0x5C] = {"jml", &CPU::JML, AddMode::AbsoluteLong, 4};
	opTable[0x5D] = {"eor", &CPU::EOR, AddMode::AbsoluteIndexedX, 4};
	opTable[0x5E] = {"lsr", &CPU::LSR, AddMode::AbsoluteIndexedX, 7};
	opTable[0x5F] = {"eor", &CPU::EOR, AddMode::AbsoluteLongIndexedX, 5};

	opTable[0x60] = {"rts", &CPU::RTS, AddMode::Implied, 6};
	opTable[0x61] = {"adc", &CPU::ADC, AddMode::DirectPageIndirectX, 6};
	opTable[0x62] = {"per", &CPU::PER, AddMode::Implied, 6};
	opTable[0x63] = {"adc", &CPU::ADC, AddMode::StackRelative, 4};
	opTable[0x64] = {"stz", &CPU::STZ, AddMode::DirectPage, 3};
	opTable[0x65] = {"adc", &CPU::ADC, AddMode::DirectPage, 3};
	opTable[0x66] = {"ror", &CPU::ROR, AddMode::DirectPage, 5};
	opTable[0x67] = {"adc", &CPU::ADC, AddMode::DirectPageIndirectLong, 6};
	opTable[0x68] = {"pla", &CPU::PLA, AddMode::Implied, 4};
	opTable[0x69] = {"adc", &CPU::ADC, AddMode::Immediate, 2};
	opTable[0x6A] = {"ror", &CPU::ROR, AddMode::Implied, 2};
	opTable[0x6B] = {"rtl", &CPU::RTL, AddMode::Implied, 6};
	opTable[0x6C] = {"jmp", &CPU::JMP, AddMode::AbsoluteIndirect, 5};
	opTable[0x6D] = {"adc", &CPU::ADC, AddMode::Absolute, 4};
	opTable[0x6E] = {"ror", &CPU::ROR, AddMode::Absolute, 6};
	opTable[0x6F] = {"adc", &CPU::ADC, AddMode::AbsoluteLong, 5};

	opTable[0x70] = {"bvs", &CPU::BVS, AddMode::Implied, 2};
	opTable[0x71] = {"adc", &CPU::ADC, AddMode::DirectPageIndirectIndexedY, 5};
	opTable[0x72] = {"adc", &CPU::ADC, AddMode::DirectPageIndirect, 5};
	opTable[0x73] = {"adc", &CPU::ADC, AddMode::StackRelativeIndirectIndexedY, 7};
	opTable[0x74] = {"stz", &CPU::STZ, AddMode::DirectPageIndexedX, 4};
	opTable[0x75] = {"adc", &CPU::ADC, AddMode::DirectPageIndexedX, 4};
	opTable[0x76] = {"ror", &CPU::ROR, AddMode::DirectPageIndexedX, 6};
	opTable[0x77] = {"adc", &CPU::ADC, AddMode::DirectPageIndirectLongIndexedY, 6};
	opTable[0x78] = {"sei", &CPU::SEI, AddMode::Implied, 2};
	opTable[0x79] = {"adc", &CPU::ADC, AddMode::AbsoluteIndexedY, 4};
	opTable[0x7A] = {"ply", &CPU::PLY, AddMode::Implied, 4};
	opTable[0x7B] = {"tdc", &CPU::TDC, AddMode::Implied, 2};
	opTable[0x7C] = {"jmp", &CPU::JMP, AddMode::AbsoluteIndexedIndirectX, 6};
	opTable[0x7D] = {"adc", &CPU::ADC, AddMode::AbsoluteIndexedX, 4};
	opTable[0x7E] = {"ror", &CPU::ROR, AddMode::AbsoluteIndexedX, 7};
	opTable[0x7F] = {"adc", &CPU::ADC, AddMode::AbsoluteLongIndexedX, 5};

	opTable[0x80] = {"bra", &CPU::BRA, AddMode::Implied, 3};
	opTable[0x81] = {"sta", &CPU::STA, AddMode::DirectPageIndirectX, 6};
	opTable[0x82] = {"brl", &CPU::BRL, AddMode::Implied, 4};
	opTable[0x83] = {"sta", &CPU::STA, AddMode::StackRelative, 4};
	opTable[0x84] = {"sty", &CPU::STY, AddMode::DirectPage, 3};
	opTable[0x85] = {"sta", &CPU::STA, AddMode::DirectPage, 3};
	opTable[0x86] = {"stx", &CPU::STX, AddMode::DirectPage, 3};
	opTable[0x87] = {"sta", &CPU::STA, AddMode::DirectPageIndirectLong, 6};
	opTable[0x88] = {"dey", &CPU::DEY, AddMode::Implied, 2};
	opTable[0x89] = {"bit", &CPU::BIT, AddMode::Immediate, 2};
	opTable[0x8A] = {"txa", &CPU::TXA, AddMode::Implied, 2};
	opTable[0x8B] = {"phb", &CPU::PHB, AddMode::Implied, 3};
	opTable[0x8C] = {"sty", &CPU::STY, AddMode::Absolute, 4};
	opTable[0x8D] = {"sta", &CPU::STA, AddMode::Absolute, 4};
	opTable[0x8E] = {"stx", &CPU::STX, AddMode::Absolute, 4};
	opTable[0x8F] = {"sta", &CPU::STA, AddMode::AbsoluteLong, 5};

	opTable[0x90] = {"bcc", &CPU::BCC, AddMode::Implied, 2};
	opTable[0x91] = {"sta", &CPU::STA, AddMode::DirectPageIndirectIndexedY, 6};
	opTable[0x92] = {"sta", &CPU::STA, AddMode::DirectPageIndirect, 5};
	opTable[0x93] = {"sta", &CPU::STA, AddMode::StackRelativeIndirectIndexedY, 7};
	opTable[0x94] = {"sty", &CPU::STY, AddMode::DirectPageIndexedX, 4};
	opTable[0x95] = {"sta", &CPU::STA, AddMode::DirectPageIndexedX, 4};
	opTable[0x96] = {"stx", &CPU::STX, AddMode::DirectPageIndexedY, 4};
	opTable[0x97] = {"sta", &CPU::STA, AddMode::DirectPageIndirectLongIndexedY, 6};
	opTable[0x98] = {"tya", &CPU::TYA, AddMode::Implied, 2};
	opTable[0x99] = {"sta", &CPU::STA, AddMode::AbsoluteIndexedY, 5};
	opTable[0x9A] = {"txs", &CPU::TXS, AddMode::Implied, 2};
	opTable[0x9B] = {"txy", &CPU::TXY, AddMode::Implied, 2};
	opTable[0x9C] = {"stz", &CPU::STZ, AddMode::Absolute, 4};
	opTable[0x9D] = {"sta", &CPU::STA, AddMode::AbsoluteIndexedX, 5};
	opTable[0x9E] = {"stz", &CPU::STZ, AddMode::AbsoluteIndexedX, 5};
	opTable[0x9F] = {"sta", &CPU::STA, AddMode::AbsoluteLongIndexedX, 5};

	opTable[0xA0] = {"ldy", &CPU::LDY, AddMode::Immediate, 2};
	opTable[0xA1] = {"lda", &CPU::LDA, AddMode::DirectPageIndirectX, 6};
	opTable[0xA2] = {"ldx", &CPU::LDX, AddMode::Immediate, 2};
	opTable[0xA3] = {"lda", &CPU::LDA, AddMode::StackRelative, 4};
	opTable[0xA4] = {"ldy", &CPU::LDY, AddMode::DirectPage, 3};
	opTable[0xA5] = {"lda", &CPU::LDA, AddMode::DirectPage, 3};
	opTable[0xA6] = {"ldx", &CPU::LDX, AddMode::DirectPage, 3};
	opTable[0xA7] = {"lad", &CPU::LDA, AddMode::DirectPageIndirectLong, 6};
	opTable[0xA8] = {"tay", &CPU::TAY, AddMode::Implied, 2};
	opTable[0xA9] = {"lda", &CPU::LDA, AddMode::Immediate, 2};
	opTable[0xAA] = {"tax", &CPU::TAX, AddMode::Implied, 2};
	opTable[0xAB] = {"plb", &CPU::PLB, AddMode::Implied, 4};
	opTable[0xAC] = {"ldy", &CPU::LDY, AddMode::Absolute, 4};
	opTable[0xAD] = {"lda", &CPU::LDA, AddMode::Absolute, 4};
	opTable[0xAE] = {"ldx", &CPU::LDX, AddMode::Absolute, 4};
	opTable[0xAF] = {"lda", &CPU::LDA, AddMode::AbsoluteLong, 5};

	opTable[0xB0] = {"bcs", &CPU::BCS, AddMode::Implied, 2};
	opTable[0xB1] = {"lda", &CPU::LDA, AddMode::DirectPageIndirectIndexedY, 5};
	opTable[0xB2] = {"lda", &CPU::LDA, AddMode::DirectPageIndirect, 5};
	opTable[0xB3] = {"lda", &CPU::LDA, AddMode::StackRelativeIndirectIndexedY, 7};
	opTable[0xB4] = {"ldy", &CPU::LDY, AddMode::DirectPageIndexedX, 4};
	opTable[0xB5] = {"lda", &CPU::LDA, AddMode::DirectPageIndexedX, 4};
	opTable[0xB6] = {"ldx", &CPU::LDX, AddMode::DirectPageIndexedY, 4};
	opTable[0xB7] = {"lda", &CPU::LDA, AddMode::DirectPageIndirectLongIndexedY, 6};
	opTable[0xB8] = {"clv", &CPU::CLV, AddMode::Implied, 2};
	opTable[0xB9] = {"lda", &CPU::LDA, AddMode::AbsoluteIndexedY, 4};
	opTable[0xBA] = {"tsx", &CPU::TSX, AddMode::Implied, 2};
	opTable[0xBB] = {"tyx", &CPU::TYX, AddMode::Implied, 2};
	opTable[0xBC] = {"ldy", &CPU::LDY, AddMode::AbsoluteIndexedX, 4};
	opTable[0xBD] = {"lda", &CPU::LDA, AddMode::AbsoluteIndexedX, 4};
	opTable[0xBE] = {"ldx", &CPU::LDX, AddMode::AbsoluteIndexedY, 4};
	opTable[0xBF] = {"lda", &CPU::LDA, AddMode::AbsoluteLongIndexedX, 5};

	opTable[0xC0] = {"cpy", &CPU::CPY, AddMode::Immediate, 2};
	opTable[0xC1] = {"cmp", &CPU::CMP, AddMode::DirectPageIndirectX, 6};
	opTable[0xC2] = {"rep", &CPU::REP, AddMode::Immediate, 3};
	opTable[0xC3] = {"cmp", &CPU::CMP, AddMode::StackRelative, 4};
	opTable[0xC4] = {"cpy", &CPU::CPY, AddMode::DirectPage, 3};
	opTable[0xC5] = {"cmp", &CPU::CMP, AddMode::DirectPage, 3};
	opTable[0xC6] = {"dec", &CPU::DEC, AddMode::DirectPage, 5};
	opTable[0xC7] = {"cmp", &CPU::CMP, AddMode::DirectPageIndirectLong, 6};
	opTable[0xC8] = {"iny", &CPU::INY, AddMode::Implied, 2};
	opTable[0xC9] = {"cmp", &CPU::CMP, AddMode::Immediate, 2};
	opTable[0xCA] = {"dex", &CPU::DEX, AddMode::Implied, 2};
	opTable[0xCB] = {"wai", &CPU::WAI, AddMode::Implied, 3};
	opTable[0xCC] = {"cpy", &CPU::CPY, AddMode::Absolute, 4};
	opTable[0xCD] = {"cmp", &CPU::CMP, AddMode::Absolute, 4};
	opTable[0xCE] = {"dec", &CPU::DEC, AddMode::Absolute, 6};
	opTable[0xCF] = {"cmp", &CPU::CMP, AddMode::AbsoluteLong, 5};

	opTable[0xD0] = {"bne", &CPU::BNE, AddMode::Implied, 2};
	opTable[0xD1] = {"cmp", &CPU::CMP, AddMode::DirectPageIndirectIndexedY, 5};
	opTable[0xD2] = {"cmp", &CPU::CMP, AddMode::DirectPageIndirect, 5};
	opTable[0xD3] = {"cmp", &CPU::CMP, AddMode::StackRelativeIndirectIndexedY, 7};
	opTable[0xD4] = {"pei", &CPU::PEI, AddMode::Implied, 6};
	opTable[0xD5] = {"cmp", &CPU::CMP, AddMode::DirectPageIndexedX, 4};
	opTable[0xD6] = {"dec", &CPU::DEC, AddMode::DirectPageIndexedX, 6};
	opTable[0xD7] = {"cmp", &CPU::CMP, AddMode::DirectPageIndirectLongIndexedY, 6};
	opTable[0xD8] = {"cld", &CPU::CLD, AddMode::Implied, 2};
	opTable[0xD9] = {"cmp", &CPU::CMP, AddMode::AbsoluteIndexedY, 4};
	opTable[0xDA] = {"phx", &CPU::PHX, AddMode::Implied, 3};
	opTable[0xDB] = {"stp", &CPU::STP, AddMode::Implied, 3};
	opTable[0xDC] = {"jml", &CPU::JML, AddMode::AbsoluteIndirectLong, 6};
	opTable[0xDD] = {"cmp", &CPU::CMP, AddMode::AbsoluteIndexedX, 4};
	opTable[0xDE] = {"dec", &CPU::DEC, AddMode::AbsoluteIndexedX, 7};
	opTable[0xDF] = {"cmp", &CPU::CMP, AddMode::AbsoluteLongIndexedX, 5};

	opTable[0xE0] = {"cpx", &CPU::CPX, AddMode::Immediate, 2};
	opTable[0xE1] = {"sbc", &CPU::SBC, AddMode::DirectPageIndirectX, 6};
	opTable[0xE2] = {"sep", &CPU::SEP, AddMode::Immediate, 3};
	opTable[0xE3] = {"sbc", &CPU::SBC, AddMode::StackRelative, 4};
	opTable[0xE4] = {"cpx", &CPU::CPX, AddMode::DirectPage, 3};
	opTable[0xE5] = {"sbc", &CPU::SBC, AddMode::DirectPage, 3};
	opTable[0xE6] = {"inc", &CPU::INC, AddMode::DirectPage, 5};
	opTable[0xE7] = {"sbc", &CPU::SBC, AddMode::DirectPageIndirectLong, 6};
	opTable[0xE8] = {"inx", &CPU::INX, AddMode::Implied, 2};
	opTable[0xE9] = {"sbc", &CPU::SBC, AddMode::Immediate, 2};
	opTable[0xEA] = {"nop", &CPU::NOP, AddMode::Implied, 2};
	opTable[0xEB] = {"xba", &CPU::XBA, AddMode::Implied, 3};
	opTable[0xEC] = {"cpx", &CPU::CPX, AddMode::Absolute, 4};
	opTable[0xED] = {"sbc", &CPU::SBC, AddMode::Absolute, 4};
	opTable[0xEE] = {"inc", &CPU::INC, AddMode::Absolute, 6};
	opTable[0xEF] = {"sbc", &CPU::SBC, AddMode::AbsoluteLong, 5};

	opTable[0xF0] = {"beq", &CPU::BEQ, AddMode::Implied, 2};
	opTable[0xF1] = {"sbc", &CPU::SBC, AddMode::DirectPageIndirectIndexedY, 5};
	opTable[0xF2] = {"sbc", &CPU::SBC, AddMode::DirectPageIndirect, 5};
	opTable[0xF3] = {"sbc", &CPU::SBC, AddMode::StackRelativeIndirectIndexedY, 7};
	opTable[0xF4] = {"pea", &CPU::PEA, AddMode::Implied, 5};
	opTable[0xF5] = {"sbc", &CPU::SBC, AddMode::DirectPageIndexedX, 4};
	opTable[0xF6] = {"inc", &CPU::INC, AddMode::DirectPageIndexedX, 6};
	opTable[0xF7] = {"sbc", &CPU::SBC, AddMode::DirectPageIndirectLongIndexedY, 6};
	opTable[0xF8] = {"sed", &CPU::SED, AddMode::Implied, 2};
	opTable[0xF9] = {"sbc", &CPU::SBC, AddMode::AbsoluteIndexedY, 4};
	opTable[0xFA] = {"plx", &CPU::PLX, AddMode::Implied, 4};
	opTable[0xFB] = {"xce", &CPU::XCE, AddMode::Implied, 2};
	opTable[0xFC] = {"jsr", &CPU::JSR, AddMode::AbsoluteIndexedIndirectX, 8};
	opTable[0xFD] = {"sbc", &CPU::SBC, AddMode::AbsoluteIndexedX, 4};
	opTable[0xFE] = {"inc", &CPU::INC, AddMode::AbsoluteIndexedX, 7};
	opTable[0xFF] = {"sbc", &CPU::SBC, AddMode::AbsoluteLongIndexedX, 5};
}

int CPU::Clock() {
	int opcycles = 0;
	if (cycles <= 0) {
		if (CheckIRQ()) {
			IRQ();
		}
		else {
			opcode = ReadMemory((regs.PB << 16) | regs.PC);
	
			if (pendingDMA) {
				DMA();
				pendingDMA = false;
			}

			//printf("\033[0;36mPC: 0x%02x%04x OP: %s %02x\033[0m\n", regs.PB, regs.PC, opTable[opcode].name, opcode);
			if (debug) {
				Debug();
			}
	
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

void CPU::EndNMI() {
	nmi &= ~0x80;
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
		else if (reg == 0x14) {	// RDDIVL
			value = quotient & 0xFF;
		}
		else if (reg == 0x15) {	// RDDIVH
			value = quotient >> 8;
		}
		else if (reg == 0x16) {	// RDMPYL
			value = product & 0xFF;
		}
		else if (reg == 0x17) {	// RDMPYH
			value = product >> 8;
		}
		else if (reg == 0x18) {	// JOY1L
			return joypad1Low;
		}
		else if (reg == 0x19) {	// JOY1H
			return joypad1High;
		}
		else if (reg == 0x1A) {	// JOY2L
			return 0;
		}
		else if (reg == 0x1B) {	// JOY2H
			return 0;
		}
		else {
			printf("\033[0;36mReading CPU: %06x\033[0m\n", address);
		}
	}
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
		else if (reg == 0x02) {	// WRMPYA
			mult1 = value;
		}
		else if (reg == 0x03) {	// WRMPYB
			mult2 = value;
			cycles += 8;

			product = mult1 * mult2;
		}
		else if (reg == 0x04) {	// WRDIVL
			divident &= 0xFF00;
			divident |= value;
		}
		else if (reg == 0x05) {	// WRDIVH
			divident &= 0x00FF;
			divident |= (value << 8);
		}
		else if (reg == 0x06) {	// WRDIVB
			divisor = value;
			cycles += 16;

			if (divisor == 0) {
				quotient = 0xFFFF;
				product = regs.P & CFlag;
			}
			else {
				quotient = divident / divisor;
				product = divident % divisor;
			}
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
				Dma[dmaChannel].count &= 0xFF00;
				Dma[dmaChannel].count |= value;
			}
			else if (reg == 0x6) {	// DASxH
				Dma[dmaChannel].count &= 0x00FF;
				Dma[dmaChannel].count |= (value << 8);
			}
			else if (reg == 0x7) {	// DASBx
				Dma[dmaChannel].indirectBank = value;
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
				Dma[dmaChannel].repeat = (value - 1) & 0x80;
				Dma[dmaChannel].lineCounter = value;
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
			if (--Dma[i].lineCounter == 0) {
				if (ReadMemory(Dma[i].tableAddress | (Dma[i].ABank << 16)) == 0) {
					Dma[i].finished = true;
					continue;
				}
				uint8_t tableEntry = ReadMemory(Dma[i].tableAddress | (Dma[i].ABank << 16));
				Dma[i].repeat = (tableEntry - 1) & 0x80;
				Dma[i].lineCounter = tableEntry;
				Dma[i].tableAddress++;
				if (Dma[i].indirect) {
					Dma[i].count = ReadMemory(Dma[i].tableAddress | (Dma[i].ABank << 16));
					Dma[i].count |= ReadMemory((Dma[i].tableAddress + 1) | (Dma[i].ABank << 16)) << 8;
					Dma[i].tableAddress += 2;
					TransferDMA(Dma[i].count, Dma[i].indirectBank, Dma[i].BAddress, Dma[i].mode, Dma[i].direction, 0, 0xFFFF);
				} else {
					TransferDMA(Dma[i].tableAddress, Dma[i].ABank, Dma[i].BAddress, Dma[i].mode, Dma[i].direction, 0, 0xFFFF);
				}
			}
			else if (Dma[i].repeat) {
				if (Dma[i].indirect) {
					TransferDMA(Dma[i].count, Dma[i].indirectBank, Dma[i].BAddress, Dma[i].mode, Dma[i].direction, 0, 0xFFFF);
				} else {
					TransferDMA(Dma[i].tableAddress, Dma[i].ABank, Dma[i].BAddress, Dma[i].mode, Dma[i].direction, 0, 0xFFFF);
				}
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
			Dma[i].repeat = (tableEntry - 1) & 0x80;
			Dma[i].lineCounter = tableEntry;
			Dma[i].tableAddress++;

			if (Dma[i].indirect) {
				Dma[i].count = ReadMemory(Dma[i].tableAddress | (Dma[i].ABank << 16));
				Dma[i].count |= ReadMemory((Dma[i].tableAddress + 1) | (Dma[i].ABank << 16)) << 8;
				Dma[i].tableAddress += 2;
				TransferDMA(Dma[i].count, Dma[i].indirectBank, Dma[i].BAddress, Dma[i].mode, Dma[i].direction, 0, 0xFFFF);
			} else {
				TransferDMA(Dma[i].tableAddress, Dma[i].ABank, Dma[i].BAddress, Dma[i].mode, Dma[i].direction, 0, 0xFFFF);
			}
		}
	}
}

void CPU::DMA() {
	for (uint8_t i = 0; i < 8; i++) {
		if (enabledDMA & (1 << i)) {
			do {
				Dma[i].count = TransferDMA(Dma[i].AAddress, Dma[i].ABank, Dma[i].BAddress, Dma[i].mode, Dma[i].direction, Dma[i].increment, Dma[i].count);
			} while (Dma[i].count);
			enabledDMA &= ~(1 << i);
			if (enabledDMA == 0x00) {
				return;
			}
		}
	}
}

int CPU::TransferDMA(uint16_t& AAddress, uint8_t Bank, uint16_t BAddress, uint8_t mode, bool direction, uint8_t increment, uint16_t count) {
	switch (mode) {
		case 0:
		{
			if (!direction) {
				WriteMemory(BAddress, ReadMemory(AAddress | (Bank << 16)));
			}
			else {
				WriteMemory(AAddress | (Bank << 16), ReadMemory(BAddress));
			}
			AAddress += (increment == 0) ? 1 : ((increment == 2) ? -1 : 0);
			if (!--count) return 0;
			break;
		}

		case 1:
		{
			if (!direction) {
				WriteMemory(BAddress, ReadMemory(AAddress | (Bank << 16)));
				if (!--count) return 0;
				WriteMemory(BAddress + 1, ReadMemory(AAddress + 1 | (Bank << 16)));
			}
			else {
				WriteMemory(AAddress | (Bank << 16), ReadMemory(BAddress));
				if (!--count) return 0;
				WriteMemory(AAddress + 1 | (Bank << 16), ReadMemory(BAddress + 1));
			}
			AAddress += (increment == 0) ? 2 : ((increment == 2) ? -2 : 0);
			if (!--count) return 0;
			break;
		}

		case 2:
		{
			if (!direction) {
				WriteMemory(BAddress, ReadMemory(AAddress | (Bank << 16)));
				if (!--count) return 0;
				WriteMemory(BAddress, ReadMemory(AAddress + 1 | (Bank << 16)));
			}
			else {
				WriteMemory(AAddress | (Bank << 16), ReadMemory(BAddress));
				if (!--count) return 0;
				WriteMemory(AAddress | (Bank << 16), ReadMemory(BAddress + 1));
			}
			AAddress += (increment == 0) ? 2 : ((increment == 2) ? -2 : 0);
			if (!--count) return 0;
			break;
		}

		case 3:
		{
			if (!direction) {
				WriteMemory(BAddress, ReadMemory(AAddress | (Bank << 16)));
				if (!--count) return 0;
				WriteMemory(BAddress, ReadMemory(AAddress + 1 | (Bank << 16)));
				if (!--count) return 0;
				WriteMemory(BAddress + 1, ReadMemory(AAddress + 2 | (Bank << 16)));
				if (!--count) return 0;
				WriteMemory(BAddress + 1, ReadMemory(AAddress + 3 | (Bank << 16)));
			}
			else {
				WriteMemory(AAddress | (Bank << 16), ReadMemory(BAddress));
				if (!--count) return 0;
				WriteMemory(AAddress | (Bank << 16), ReadMemory(BAddress + 1));
				if (!--count) return 0;
				WriteMemory(AAddress + 1 | (Bank << 16), ReadMemory(BAddress + 2));
				if (!--count) return 0;
				WriteMemory(AAddress + 1 | (Bank << 16), ReadMemory(BAddress + 3));
			}
			AAddress += (increment == 0) ? 4 : ((increment == 2) ? -4 : 0);
			if (!--count) return 0;
			break;
		}

		case 4:
		{
			if (!direction) {
				WriteMemory(BAddress, ReadMemory(AAddress | (Bank << 16)));
				if (!--count) return 0;
				WriteMemory(BAddress + 1, ReadMemory(AAddress + 1 | (Bank << 16)));
				if (!--count) return 0;
				WriteMemory(BAddress + 2, ReadMemory(AAddress + 2 | (Bank << 16)));
				if (!--count) return 0;
				WriteMemory(BAddress + 3, ReadMemory(AAddress + 3 | (Bank << 16)));
			}
			else {
				WriteMemory(AAddress | (Bank << 16), ReadMemory(BAddress));
				if (!--count) return 0;
				WriteMemory(AAddress + 1 | (Bank << 16), ReadMemory(BAddress + 1));
				if (!--count) return 0;
				WriteMemory(AAddress + 2 | (Bank << 16), ReadMemory(BAddress + 2));
				if (!--count) return 0;
				WriteMemory(AAddress + 3 | (Bank << 16), ReadMemory(BAddress + 3));
			}
			AAddress += (increment == 0) ? 4 : ((increment == 2) ? -4 : 0);
			if (!--count) return 0;
			break;
		}

		default:
			printf("Mode: %d\n", mode);
			assert(false && "DMA Mode not handled");
	}
	return count;
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
			uint16_t indirectAddres = ReadMemory((regs.PB << 16) | regs.PC++);
			indirectAddres |= ReadMemory((regs.PB << 16) | regs.PC++) << 8;
			
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
		regs.DB = ReadMemory((regs.PB << 16) | regs.PC++);
		srcBank = ReadMemory((regs.PB << 16) | regs.PC++);
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
		regs.DB = ReadMemory((regs.PB << 16) | regs.PC++);
		srcBank = ReadMemory((regs.PB << 16) | regs.PC++);
		regs.PC -= 2;
		movActive = true;
	}
	assert(!(regs.P & XFlag) && "MVP XFlag");
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
			SetFlag(CFlag, (uint8_t)result <= regs.A.L);

			regs.A.L = result;

		} else {
			uint8_t result = regs.A.L - fetched - carry;
			CheckNFlag8Bits(result);
			CheckZFlag8Bits(result);

			SetFlag(CFlag, result <= regs.A.L);

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
			SetFlag(CFlag, (uint16_t)result <= regs.A.HL);

			regs.A.HL = result;

		} else {
			uint16_t result = regs.A.HL - fetched - carry;
			CheckNFlag16Bits(result);
			CheckZFlag16Bits(result);

			SetFlag(CFlag, result <= regs.A.HL);

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
