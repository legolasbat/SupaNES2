#include "SupaNES.h"

#include <assert.h>
#include <stdio.h>

SupaNES::SupaNES() {
	cpu = new CPU(this);
	ppu = new PPU(this);
	apu = new APU(this);
	cart = new Cartridge(this);
	
	RomLoaded = false;

	memoryBank = 0;
	memoryPage = 0;

	for (int i = 0; i < 0x20000; i++) {
		WRAM[i] = 0;
	}
}

int64_t scheduler_CPU_SPC700 = 0;
const uint32_t CPU_MASTER_CLOCK = 21477270;
const uint32_t SPC700_MASTER_CLOCK = 24576000;

int SupaNES::Clock() {
	int cycles = 0;
	if (scheduler_CPU_SPC700 >= 0) {
		cycles = cpu->Clock();
		for (int i = 0; i < cycles; i++) {
			ppu->Clock();
		}
		scheduler_CPU_SPC700 -= cycles * SPC700_MASTER_CLOCK;
	}
	else {
		cycles = apu->Clock();
		scheduler_CPU_SPC700 += cycles * 24 * CPU_MASTER_CLOCK;
	}
	return cycles;
}

uint8_t SupaNES::ReadMemory(uint32_t address) {
	memoryBank = (address & 0x00FF0000) >> 16;
	memoryPage = (address & 0x0000FF00) >> 8;

	uint8_t value = 0;

	if (memoryBank < 0x40) {
		value = ReadMemoryQ1(address);
	}
	else if (memoryBank < 0x80) {
		value = ReadMemoryQ2(address);
	}
	else if (memoryBank < 0xC0) {
		value = ReadMemoryQ3(address);
	}
	else {
		value = ReadMemoryQ3(address);
	}

	return value;
}

void SupaNES::WriteMemory(uint32_t address, uint8_t value) {
	memoryBank = (address & 0x00FF0000) >> 16;
	memoryPage = (address & 0x0000FF00) >> 8;

	if (memoryBank < 0x40) {
		WriteMemoryQ1(address, value);
	}
	else if (memoryBank < 0x80) {
		WriteMemoryQ2(address, value);
	}
	else if (memoryBank < 0xC0) {
		WriteMemoryQ3(address, value);
	}
	else {
		WriteMemoryQ4(address, value);
	}
}

uint8_t SupaNES::ReadMemoryQ1(uint32_t address) {
	uint8_t value = 0;

	// WRAM
	if (memoryPage < 0x20) {
		value = WRAM[address % 0x2000];
	}
	else if (memoryPage == 0x21) {
		//TODO: Make PPU, APU and WRAM
		if ((address & 0xFF) <= 0x3F) {
			return ppu->ReadPPU(address);
		}
		else if ((address & 0xFF) == 0x40) {
			return apu->ReadAPUPorts(address);
		}
		else if ((address & 0xFF) == 0x41) {
			return apu->ReadAPUPorts(address);
		}
		else if ((address & 0xFF) == 0x42) {
			return apu->ReadAPUPorts(address);
		}
		else if ((address & 0xFF) == 0x43) {
			return apu->ReadAPUPorts(address);
		}
		else if ((address & 0xFF) == 0x80) {
			value = WRAM[WRAMAddress++];
			WRAMAddress %= 0x20000;
			return value;
		}
		else if ((address & 0xFF) > 0x80) {
			// Open Bus
			return 0;
		}
		printf("Reading %06X\n", address);
		assert(false && "Reading Q1 Page 21");
	}
	else if (memoryPage == 0x40) {
		// Reading Joypad register
		if ((address & 0xFF) == 0x16) {
			if (joypad1Latch >= 16) {
				return 1;
			}
			if (latchController) {
				return cpu->joypad1High >> 7;
			}

			switch (joypad1Latch++) {
			case  0: return (cpu->joypad1High >> 7) & 1;
			case  1: return (cpu->joypad1High >> 6) & 1;
			case  2: return (cpu->joypad1High >> 5) & 1;
			case  3: return (cpu->joypad1High >> 4) & 1;
			case  4: return (cpu->joypad1High >> 3) & 1;
			case  5: return (cpu->joypad1High >> 2) & 1;
			case  6: return (cpu->joypad1High >> 1) & 1;
			case  7: return (cpu->joypad1High >> 0) & 1;
			case  8: return (cpu->joypad1Low >> 7) & 1;
			case  9: return (cpu->joypad1Low >> 6) & 1;
			case 10: return (cpu->joypad1Low >> 5) & 1;
			case 11: return (cpu->joypad1Low >> 4) & 1;
			default: return 0;
			}
		}
		else if ((address & 0xFF) == 0x17) {
			if (joypad2Latch >= 16) {
				return 1;
			}
			if (latchController) {
				return 0;
				//return cpu->joypad2High >> 7;
			}

			switch (joypad2Latch++) {
			//case  0: return (cpu->joypad2High >> 7) & 1;
			//case  1: return (cpu->joypad2High >> 6) & 1;
			//case  2: return (cpu->joypad2High >> 5) & 1;
			//case  3: return (cpu->joypad2High >> 4) & 1;
			//case  4: return (cpu->joypad2High >> 3) & 1;
			//case  5: return (cpu->joypad2High >> 2) & 1;
			//case  6: return (cpu->joypad2High >> 1) & 1;
			//case  7: return (cpu->joypad2High >> 0) & 1;
			//case  8: return (cpu->joypad2Low >> 7) & 1;
			//case  9: return (cpu->joypad2Low >> 6) & 1;
			//case 10: return (cpu->joypad2Low >> 5) & 1;
			//case 11: return (cpu->joypad2Low >> 4) & 1;
			default: return 0;
			}
		}
		else {
			// Open Bus
			value = 0;
		}
	}
	else if (memoryPage == 0x42 || memoryPage == 0x43) {
		value = cpu->ReadCPU(address);
	}
	else if (memoryPage >= 0x80) {
		value = cart->ReadRom(address);
	}
	return value;
}

uint8_t SupaNES::ReadMemoryQ2(uint32_t address) {
	uint8_t value = 0;

	// WRAM
	if (memoryBank == 0x7E || memoryBank == 0x7F) {
		value = WRAM[address % 0x20000];
	}
	else {
		value = cart->ReadRom(address);
	}
	return value;
}

uint8_t SupaNES::ReadMemoryQ3(uint32_t address) {
	uint8_t value = 0;

	// WRAM
	if (memoryPage < 0x20) {
		value = WRAM[address % 0x2000];
	}
	else if (memoryPage == 0x21) {
		//TODO: Make PPU, APU and WRAM
		if ((address & 0xFF) <= 0x3F) {
			return ppu->ReadPPU(address);
		}
		else if ((address & 0xFF) == 0x40) {
			return apu->ReadAPUPorts(address);
		}
		else if ((address & 0xFF) == 0x41) {
			return apu->ReadAPUPorts(address);
		}
		else if ((address & 0xFF) == 0x42) {
			return apu->ReadAPUPorts(address);
		}
		else if ((address & 0xFF) == 0x43) {
			return apu->ReadAPUPorts(address);
		}
		else if ((address & 0xFF) == 0x80) {
			value = WRAM[WRAMAddress++];
			WRAMAddress %= 0x20000;
			return value;
		}
		printf("Reading %06X\n", address);
		assert(false && "Reading Q1 Page 21");
	}
	else if (memoryPage == 0x40) {
		// Reading Joypad register
		//printf("Read %06X\n", address);
		//assert(false && "Reading Q1 Page 40");
	}
	else if (memoryPage == 0x42 || memoryPage == 0x43) {
		value = cpu->ReadCPU(address);
	}
	else if (memoryPage >= 0x80) {
		value = cart->ReadRom(address);
	}
	return value;
}

uint8_t SupaNES::ReadMemoryQ4(uint32_t address)
{
	uint8_t value = 0;

	assert(!(memoryBank >= 0xF0 && memoryPage < 0x80));
	value = cart->ReadRom(address);

	return value;
}

void SupaNES::WriteMemoryQ1(uint32_t address, uint8_t value) {
	// WRAM
	if (memoryPage < 0x20) {
		WRAM[address % 0x2000] = value;
	}
	else if (memoryPage == 0x21) {
		//TODO: Make PPU, APU and WRAM
		// PPU
		if ((address & 0xFF) <= 0x3F) {
			ppu->WritePPU(address, value);
		}
		// APU
		else if ((address & 0xFF) <= 0x43) {
			apu->WriteCPUPorts(address, value);
		}
		// WRAM
		else if ((address & 0xFF) == 0x80) {	// Write/Read
			WRAM[WRAMAddress] = value;
			WRAMAddress++;
			WRAMAddress %= 0x20000;
		}
		else if ((address & 0xFF) == 0x81) {	// Low Address
			WRAMAddress &= 0x1FF00;
			WRAMAddress |= value;
			WRAMAddress %= 0x20000;
		}
		else if ((address & 0xFF) == 0x82) {	// Page Address
			WRAMAddress &= 0x100FF;
			WRAMAddress |= value << 8;
			WRAMAddress %= 0x20000;
		}
		else if ((address & 0xFF) == 0x83) {	// Bank Address
			WRAMAddress &= 0xFFFF;
			WRAMAddress |= value << 16;
			WRAMAddress %= 0x20000;
		} else {
			printf("Write Page 21: %06X\n", address);
		}
	}
	else if (memoryPage == 0x40) {
		// Writting Joypad register
		if ((address & 0xFF) == 0x16) {			// JOYSER0
			if (latchController != (value & 0x1)) {
				latchController = (value & 0x1);
				joypad1Latch = 0;
				joypad2Latch = 0;
			}
		}
		else {
			printf("Write Page 40: %06X\n", address);
			assert(false);
		}
	}
	// CPU
	else if (memoryPage == 0x42 || memoryPage == 0x43) {
		cpu->WriteCPU(address, value);
	}
	else if (memoryPage >= 0x80) {
		printf("Write %06X\n", address);
		assert(false && "Writting Q1 Cart");
	}
}

void SupaNES::WriteMemoryQ2(uint32_t address, uint8_t value) {
	// WRAM
	if (memoryBank == 0x7E || memoryBank == 0x7F) {
		WRAM[address % 0x20000] = value;
	}
	else {
		if (memoryPage < 0x80) {
			cart->WriteRom(address, value);
		} else {
			printf("%06x\n", address);
			assert(false && "Writting ROM Q2");
		}
	}
}

void SupaNES::WriteMemoryQ3(uint32_t address, uint8_t value) {
	// WRAM
	if (memoryPage < 0x20) {
		WRAM[address % 0x2000] = value;
	}
	else if (memoryPage == 0x21) {
		//TODO: Make PPU, APU and WRAM
		// PPU
		if ((address & 0xFF) <= 0x3F) {
			ppu->WritePPU(address, value);
		}
		// APU
		else if ((address & 0xFF) <= 0x43) {
			apu->WriteCPUPorts(address, value);
		}
		// WRAM
		else if ((address & 0xFF) == 0x80) {	// Write/Read
			WRAM[WRAMAddress] = value;
			WRAMAddress++;
			WRAMAddress %= 0x20000;
		}
		else if ((address & 0xFF) == 0x81) {	// Low Address
			WRAMAddress &= 0x1FF00;
			WRAMAddress |= value;
			WRAMAddress %= 0x20000;
		}
		else if ((address & 0xFF) == 0x82) {	// Page Address
			WRAMAddress &= 0x100FF;
			WRAMAddress |= value << 8;
			WRAMAddress %= 0x20000;
		}
		else if ((address & 0xFF) == 0x83) {	// Bank Address
			WRAMAddress &= 0xFFFF;
			WRAMAddress |= value << 16;
			WRAMAddress %= 0x20000;
		}
		else {
			printf("Write Page 21: %06X\n", address);
		}
	}
	else if (memoryPage == 0x40) {
		// Writting Joypad register
		printf("Write Page 40: %06X\n", address);
	}
	// CPU
	else if (memoryPage == 0x42 || memoryPage == 0x43) {
		cpu->WriteCPU(address, value);
	}
	else if (memoryPage >= 0x80) {
		printf("Write %06X\n", address);
		//assert(false && "Writting Q1 Cart");
	}
}

void SupaNES::WriteMemoryQ4(uint32_t address, uint8_t value) {
	// SRAM
	if (memoryBank >= 0xF0 && memoryPage < 0x80) {
		cart->WriteRom(address, value);
	}
}

bool SupaNES::InsertROM(const char* filepath) {
	if (RomLoaded) {
		cpu->~CPU();
		cpu = new CPU(this);
		ppu->~PPU();
		ppu = new PPU(this);
		apu->~APU();
		apu = new APU(this);
		cart->~Cartridge();
		cart = new Cartridge(this);
		RomLoaded = false;
	}

	uint16_t excVectors[16];
	if (!cart->LoadROM(filepath, excVectors)) {
		return false;
	}

	cpu->SetExcVectors(excVectors);
	
	RomLoaded = true;

	return true;
}
