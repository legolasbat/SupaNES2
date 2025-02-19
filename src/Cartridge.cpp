#include "Cartridge.h"
#include "SupaNES.h"

#include <stdio.h>
#include <stdlib.h>

Cartridge::Cartridge(SupaNES* supaNes) {
	//TODO: Init variables
	memory = supaNes;
}

bool Cartridge::LoadROM(const char* filepath, uint16_t* excVectors) {
	FILE* file = fopen(filepath, "rb");
	
	if(file == NULL){
		printf("Failed to open file: %s\n", filepath);
		return false;
	}

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	printf("Size: %ld bytes\n", fileSize);

	int offset = 0;
	// Check SMD header
	int remainder = fileSize % 1024;
	if(remainder == 512){
		printf("There is a SMD Header\n");
		offset = 512;
	}
	else if(remainder == 0){
		printf("There is not a SMD Header\n");
	}
	else{
		printf("Invalid ROM\n");
		return false;
	}
	
	// Differ LoROM and HiROM
	int headerDir = 0x7FC0 + offset;
	int checksumDir = headerDir + 0x1C;
	fseek(file, checksumDir, SEEK_SET);
	uint16_t checksumCom = 0;
	uint16_t checksum = 0;
	fread(&checksumCom, sizeof(uint16_t), 1, file);
	fread(&checksum, sizeof(uint16_t), 1, file);
	if((checksumCom ^ checksum) == 0xFFFF || fileSize < 0xFFC0){
		printf("Correct checksum. LoROM\n");
	}
	else{
		headerDir = 0xFFC0 + offset;
		checksumDir = headerDir + 0x1C;
		fseek(file, checksumDir, SEEK_SET);
		checksumCom = 0;
		checksum = 0;
		fread(&checksumCom, sizeof(uint16_t), 1, file);
		fread(&checksum, sizeof(uint16_t), 1, file);
		if((checksumCom ^ checksum) == 0xFFFF){
			printf("Correct checksum. HiROM\n");
		}
		else{
			printf("ROM type not handled.\n");
			return false;
		}
	}

	// Header Info
	fseek(file, headerDir, SEEK_SET);

	// Title
	char t;
	printf("- ROM Title: ");
	for(int i = 0; i <= 0x14; i++){
		fread(&t, sizeof(char), 1, file);
		printf("%c", t);
	}
	printf("\n");
	// Cartridge Version 2
	if(t == 0){
		printf("Cartridge Ver.2 not handled.\n");
		return false;
	}

	// Mapping Mode and Speed
	uint8_t mapMode = 0;
	fread(&mapMode, sizeof(uint8_t), 1, file);
	printf("- Speed: ");
	if((mapMode & 0x10) == 0x10){
		printf("FastROM\n");
	}
	else{
		printf("SlowROM\n");
	}
	// TODO: check mapping mode
	
	// Cartridge type
	uint8_t cartType = 0;
	fread(&cartType, sizeof(uint8_t), 1, file);
	if(!CheckCartridgeType(&cartType)){
		return false;
	}
	
	// ROM Size
	uint8_t romSize = 0;
	fread(&romSize, sizeof(uint8_t), 1, file);
	printf("- ROM SIze: ");
	int romSizeByte = (1 << romSize) * 1024;
	if (romSizeByte != fileSize) {
		romSizeByte = fileSize;
	}
	printf("%d bytes\n", romSizeByte);

	// RAM Size
	uint8_t ramSize = 0;
	fread(&ramSize, sizeof(uint8_t), 1, file);
	printf("- RAM Size: ");
	int ramSizeByte = 0;
	if(ramSize != 0){
		ramSizeByte = (1 << ramSize) * 1024;
	}
	printf("%d bytes\n", ramSizeByte);

	// Region
	uint8_t region = 0;
	fread(&region, sizeof(uint8_t), 1, file);
	if(!CheckRegion(&region)){
		return false;
	}

	// Dev ID
	// Null read to step forward
	uint8_t devID = 0;
	fread(&devID, sizeof(uint8_t), 1, file);
	if(devID == 0x33){
		printf("Cartridge Ver. 3 not handled.\n");
		return false;
	}

	// Version
	uint8_t version = 0;
	fread(&version, sizeof(uint8_t), 1, file);
	printf("- Version: 1.%d\n", version);

	// Checksums (already checked)
	uint32_t checksums = 0;
	fread(&checksums, sizeof(uint32_t), 1, file);

	// Interrupt Vectors
	fread(excVectors, sizeof(uint16_t), 16, file);

	for (int i = 0; i < 16; i++) {
		printf("Vector %d: 0x%04x\n", i, excVectors[i]);
	}
	//TODO: Load Vectors in memory

	ROM = (uint8_t*)malloc(sizeof(uint8_t) * romSizeByte);
	fseek(file, offset, SEEK_SET);
	fread(ROM, sizeof(uint8_t), romSizeByte, file);
	ROMSize = romSizeByte;

	fclose(file);
	return true;
}

bool Cartridge::CheckCartridgeType(const uint8_t* cartType){
	printf("- Chipset: ");
	switch (*cartType) {
		case 0b0000:
			printf("ROM ONLY\n");
		break;
		case 0b0001:
			printf("ROM + SRAM\n");
		break;
		case 0b0010:
			printf("ROM + SRAM + BATTERY\n");
		break;
		case 0b0011:
			printf("ROM + CO-CPU\n");
		break;
		case 0b0100:
			printf("ROM + CO-CPU + SRAM\n");
		break;
		case 0b0101:
			printf("ROM + CO-CPU + SRAM + BATTERY\n");
		break;
		case 0b0110:
			printf("ROM + CO-CPU + BATTERY\n");
		break;
		default:
			printf("CHIPSET UNKNOWN\n");
			return false;
		break;
	}
	if(*cartType > 0b0010){
		printf("COPROCESSOR NOT HANDLED\n");
		return false;
	}
	return true;
}

bool Cartridge::CheckRegion(const uint8_t* region){
	printf("- Region: ");
	switch (*region) {
		case 0x00:
			printf("Japan\n");
		break;
		case 0x01:
			printf("USA\n");
		break;
		case 0x02:
			printf("Europe\n");
		break;
		case 0x03:
			printf("Sweden\n");
		break;
		case 0x04:
			printf("Finland\n");
		break;
		case 0x05:
			printf("Denmark\n");
		break;
		case 0x06:
			printf("France\n");
		break;
		case 0x07:
			printf("Netherlands\n");
		break;
		case 0x08:
			printf("Spain\n");
		break;
		case 0x09:
			printf("Germany\n");
		break;
		case 0x0A:
			printf("Italy\n");
		break;
		case 0x0B:
			printf("China\n");
		break;
		case 0x0C:
			printf("Indonesia\n");
		break;
		case 0x0D:
			printf("South Korea\n");
		break;
		case 0x0E:
			printf("Common/International\n");
		break;
		case 0x0F:
			printf("Canada\n");
		break;
		case 0x10:
			printf("Brazil\n");
		break;
		case 0x11:
			printf("Australia\n");
		break;
		default:
			printf("UNKNOWN\n");
			return false;
		break;
	}
	return true;
}

uint8_t Cartridge::ReadRom(uint32_t address) {
	uint8_t bank = (address & 0x00FF0000) >> 16;
	uint8_t page = (address & 0x0000FF00) >> 8;

	uint8_t value = 0;

	// Check bank range
	if (bank != 0x7E && bank != 0x7F) {
		// Check page range
		// Check top half
		if (page >= 0x80) {
			// ROM
			if (bank >= 0x80) {
				//TODO: ROM Mirror
			}
			// Mirror ROM
			else {
				value = ROM[(((address & 0xFF8000) - 0x8000) / 2) + (address & 0x7FFF)];
			}
		}
		// Check bottom half
		else if (page < 0x80) {
			// SRAM 2Q
			if (bank >= 0x40 && bank < 0x7E) {
				//TODO: Read SRAM 2Q
			}
			// ROM bottom half 4Q
			else if (bank >= 0xC0 && bank < 0xF0) {
				//TODO: Read ROM Mirror Q4
			}
			// SRAM 4Q
			else if (bank >= 0xF0) {
				//TODO: Read SRAM 4Q
			}
		}
	}

	return value;
}

void Cartridge::WriteRom(uint32_t address, uint8_t value) {
	//TODO: Cartridge SRAM
}

Cartridge::~Cartridge() {
	free(ROM);
}
