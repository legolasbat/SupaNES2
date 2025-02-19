#pragma once

#include <stdint.h>

class SupaNES;

class PPU {
	public:
		SupaNES* memory;
		
		PPU(SupaNES* supaNes);

		void Clock();

		void WritePPU(uint32_t address, uint8_t value);

		int width = 256;
		int height = 224;
		uint16_t pixels[256 * 224];

		bool frameCompleted;
		uint16_t* GetFrame();
		void SetFrame(void* texturePixels, int pitch);

		int HCounter;
		int VCounter;
		int frameCounter;

	private:

		void SetPixel(int X, int Y);
		void SetBGPixel(int X, int Y, uint8_t BGAddress, uint8_t BGIndex, int bpp);
		void SetBGDrop(int X, int Y, bool forceBlank = false);

		uint16_t GetColor2BPP(uint16_t charAddress, uint8_t palette, uint8_t HShift, int BGIndex);

		uint8_t inidisp;		// INIDISP							0x2100

		uint16_t OAM[0x110];
		uint16_t OAMAddress;	// OAM Address						0x2102/3
		bool OAMFlipflop;

		uint8_t BG1TileAddress;	// BG1 Tilemap Address and Size		0x2107
		uint8_t BG2TileAddress;	// BG2 Tilemap Address and Size		0x2108
		uint8_t BG3TileAddress;	// BG3 Tilemap Address and Size		0x2109
		uint8_t BG4TileAddress;	// BG4 Tilemap Address and Size		0x210A
		
		uint8_t BGAddress1_2;	// BG1 and 2 Chr Address			0x210B
		uint8_t BGAddress3_4;	// BG3 and 4 Chr Address			0x210C

		uint8_t VMain;			// Video Port Control				0x2115
		uint16_t VRAM[0x8000];
		uint16_t VRAMAddress;	// VRAM Address						0x2116/7

		uint8_t BGMode;			// BG Mode and Character Size		0x2105
		
		uint16_t CGRAM[0x100];
		uint8_t CGRAMAddress;
		bool CGRAMFlipflop;

		uint8_t TM;				// Main Screen Designation			0x212C
};
