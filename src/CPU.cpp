#include "CPU.h"

#include "SupaNES.h"

#include <stdio.h>

CPU::CPU(SupaNES* supaNes) {
	memory = supaNes;

	registers.PC = 0x8000;

	registers.A = 0;
	registers.X = 0;
	registers.Y = 0;

	registers.SP = 0x01FF;

	registers.DP = 0;

	registers.PB = 0;

	registers.DB = 0;

	registers.P = 0x34;
}

int CPU::Clock() {
	printf("PC: 0x%02x%04x\n", registers.PB, registers.PC++);
	uint8_t opcode = ReadMemory((registers.PB << 16) | registers.PC, true);
	registers.PC++;

	printf("OP: %02x\n", opcode);
	return 0;
}

uint8_t CPU::ReadMemory(uint32_t address, bool isLong) {
	if (isLong) {
		return memory->ReadMemory(address);
	}
	return memory->ReadMemory((registers.DB << 16) | address);
}

void CPU::SetExcVectors(uint16_t vectors[16]) {
	excVectors.COPVector = vectors[2];
	excVectors.BRKVector = vectors[3];
	excVectors.NMIVector = vectors[5];
	excVectors.IRQVector = vectors[7];

	excVectors.ResetVector = vectors[14];

	registers.PC = excVectors.ResetVector;

	printf("PC: 0x%04x\n", registers.PC);
}
