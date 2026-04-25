#pragma once

#include <stdint.h>

class SupaNES;

class PPU {
	public:
		SupaNES* memory;

		// Screen Width
		static constexpr int width = 256;
		// Screen Height
		static constexpr int height = 224;

		bool frameCompleted;
		uint8_t masterClockCounter = 2;

		// Layer Display Settings
		bool BG1Display = true;
		bool BG2Display = true;
		bool BG3Display = true;
		bool BG4Display = true;
		bool ObjDisplay = true;

		bool latchCounter;
		int HCounter;		// Horizontal Scanline
		int VCounter;		// Vertical Scanline
		int frameCounter;	// Number of current frame

		struct Pixel {
			uint16_t color = 0;
			uint8_t priority : 1;
			uint8_t id : 3;
			uint8_t palette : 4;
		};
		// Pixel Info for layers
		Pixel mainPixelPipeline[6];
		// Pixel Info for sublayers
		Pixel subPixelPipeline[6];
		
		PPU(SupaNES* supaNes);

		void Clock();

		uint8_t ReadPPU(uint32_t address);
		void WritePPU(uint32_t address, uint8_t value);

		void SetFrame(void* texturePixels, int pitch);

		void GetMode7BG(void* texturePixels, int pitch) const;

		const uint16_t* GetVRAM() const { return VRAM; }
		const uint16_t* GetCGRAM() const { return CGRAM; }

	private:

		struct OBJECT {
			uint16_t posX = 0xFF;
			uint16_t maxX = 0xFF;
			uint8_t posY = 0xE0;
			uint8_t maxY = 0xE0;
			uint16_t tileID : 9;
			uint16_t priority : 2;
			uint16_t palette : 3;
			uint16_t flipX : 1;
			uint16_t flipY : 1;
			uint8_t clipping : 2;
			uint8_t size : 1;

			OBJECT() {};

			OBJECT(const OBJECT&) = delete;
			OBJECT& operator=(const OBJECT&) = delete;
		};

		uint8_t ObjSizes[2][8];

		OBJECT objs[128];
		int objsToDisplay[128];
		int lastObjDrawn = -1;
		uint16_t firstObj[2] = { 0xFFFF, 0xFFFF };
		uint16_t lastObj[2] = { 0, 0 };

		uint8_t inidisp;		// INIDISP							0x2100

		uint8_t OBSEL;			// Object Size & Character Address	0x2101

		uint16_t OAMAddress;	// OAM Address						0x2102/3
		uint16_t InternalOAMAddress;
		bool WrittenInternalOAMAddress = false;
		uint8_t OAM_Lsb = 0;

		uint8_t BGMode;			// BG Mode and Character Size		0x2105
	
		uint8_t mosaic;			// Mosaic - Screen Pixelation		0x2106

		uint8_t BG1TileAddress;	// BG1 Tilemap Address and Size		0x2107
		uint8_t BG2TileAddress;	// BG2 Tilemap Address and Size		0x2108
		uint8_t BG3TileAddress;	// BG3 Tilemap Address and Size		0x2109
		uint8_t BG4TileAddress;	// BG4 Tilemap Address and Size		0x210A
		
		uint8_t BGAddress1_2;	// BG1 and 2 Chr Address			0x210B
		uint8_t BGAddress3_4;	// BG3 and 4 Chr Address			0x210C
								
		uint8_t BG_Old_1 = 0;
		uint8_t BG_Old_2 = 0;
		uint16_t BG1HScroll;	// BG1 Horizontal Scroll and M7HOFS	0x210D
		int16_t M7HOFS;
		uint16_t BG1VScroll;	// BG1 Vertical Scroll and M7VOFS	0x210E
		int16_t M7VOFS;

		uint16_t BG2HScroll;	// BG2 Horizontal Scroll			0x210F
		uint16_t BG2VScroll;	// BG2 Vertical Scroll				0x2110
		
		uint16_t BG3HScroll;	// BG3 Horizontal Scroll			0x2111
		uint16_t BG3VScroll;	// BG3 Vertical Scroll				0x2112
		
		uint16_t BG4HScroll;	// BG4 Horizontal Scroll			0x2113
		uint16_t BG4VScroll;	// BG4 Vertical Scroll				0x2114
		
		uint8_t VMain;			// Video Port Control				0x2115
		uint16_t VRAMAddress;	// VRAM Address						0x2116/7
		bool VRAMAddressReset = false;

		uint8_t M7SEL;			// Mode 7 Settings					0x211A

		uint8_t prevM = 0;
		int16_t M7A;			// Mode 7 Matrix A					0x211B
		int16_t M7B;			// Mode 7 Matrix B					0x211C
		int16_t M7C;			// Mode 7 Matrix C					0x211D
		int16_t M7D;			// Mode 7 Matrix D					0x211E
		int16_t M7X;			// Mode 7 Center X					0x211F
		int16_t M7Y;			// Mode 7 Center Y					0x2120

		uint8_t CGRAMAddress;	// CGRAM Address					0x2121
		bool CGRAMFlipflop;

		uint8_t W12SEL;			// Window Mask Settings for BG1 & 2	0x2123
		uint8_t W34SEL;			// Window Mask Settings for BG3 & 4	0x2124
		uint8_t WOBJSEL;		// Window Mask Settings for OBJ & C	0x2125

		uint8_t window1Left;	// WH0								0x2126
		uint8_t window1Right;	// WH1								0x2127
		uint8_t window2Left;	// WH2								0x2128
		uint8_t window2Right;	// WH3								0x2129
		uint8_t WBGLOG;			// Window mask logic for BGs		0x212A
		uint8_t WOBJLOG;		// Window mask logic for OBJs and C	0x212B

		uint8_t TM;				// Main Screen Designation			0x212C
		uint8_t TS;				// Subscreen Designation			0x212D
		uint8_t TMW;			// Window Mask Designation for Main	0x212E
		uint8_t TSW;			// Window Mask Designation for Sub	0x212F

		uint8_t CGWSEL;			// Color Addition Select			0x2130
		uint8_t CGADSUB;		// Color math designation			0x2131

		uint16_t fixedColor;	// Fixed Color Data					0x2132

		bool overscan = false;	// Screen Mode/Video Select			0x2133

		int32_t multResult;		// Signed Multiply Result			0x2134/5/6

		// Flipflops for 0x213C/D
		bool OPHCTFlipflop = false;
		bool OPVCTFlipflop = false;

		uint16_t VRAM[0x8000];
		uint16_t CGRAM[0x100];

		// Colors of Screen
		uint16_t pixels[width * height];

		void SetPixel(int X, int Y);
		void SetBGPixel(int X, int Y, uint8_t BGAddress, uint8_t BGIndex, uint8_t bpp,
			uint8_t XSize, uint8_t YSize,
			uint16_t HScroll, uint16_t VScroll);
		void SetOBJPixel(int X, int Y);

		void SetBG1PixelMode7(int X, int Y);
		void CalculateMode7Coor(int& X, int& Y) const;

		uint8_t GetColorBPPForBG(uint8_t BGIndex) const;
		uint16_t GetColor2BPP(uint16_t charAddress, uint8_t palette, uint8_t HShift, uint8_t BGIndex) const;
		uint16_t GetColor4BPP(uint16_t charAddress, uint8_t palette, uint8_t HShift) const;
		uint16_t GetColor8BPP(uint16_t charAddress, uint8_t palette, uint8_t HShift) const;

		void OBJReloadSize();
		void GetOBJsToDisplay();

		bool IsOBJOnScreen(int X, int Y, PPU::OBJECT& obj, uint8_t currentPriority, int& index);

		uint16_t ColorMathPixel(uint16_t palette, Pixel subpixel) const;

		bool IsInsideWindow(int X, uint8_t BGIndex, uint8_t enableReg) const;

		Pixel GetPixelByPriority(Pixel* pixels) const;
};
