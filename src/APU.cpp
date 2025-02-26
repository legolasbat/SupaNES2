#include "APU.h"
#include <cstdint>

APU::APU(SupaNES* supaNes) {
	memory = supaNes;

	PC = 0xFFC0;	// Point to boot ROM

	A = 0;
	X = 0;
	Y = 0;

	// TODO: Load boot ROM
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

	return APUPorts[index];
}

void APU::WriteCPUPorts(uint8_t address, uint8_t value) {
	uint8_t index = address & 0x3;

	CPUPorts[index] = value;
}
