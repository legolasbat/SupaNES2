#pragma once

#include <stdint.h>
#include <stdio.h>

class SupaNES;

class Cartridge {
public:
  SupaNES *memory;

  uint8_t *ROM = nullptr;
  int ROMSize;

  uint8_t *RAM = nullptr;
  int RAMSize;

  enum Layout { LoROM, HiROM };

  Layout layout;

  Cartridge(SupaNES *supaNes);
  ~Cartridge();

  bool LoadROM(const char *filepath, uint16_t *excVectors);

  uint8_t ReadRom(uint32_t address);
  void WriteRom(uint32_t address, uint8_t value);

private:
  FILE *ramFile = NULL;

  bool CheckCartridgeType(const uint8_t *cartType);
  bool CheckRegion(const uint8_t *region);

  bool GetRAMFile(const char *filepath, bool &isCreated);
};
