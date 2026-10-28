#pragma once

#include <cstdint>

#include <SDL3/SDL_audio.h>

class APU;

class DSP {
public:
  DSP();
  ~DSP();

  void SetMemory(APU *apu) { memory = apu; }

  void Tick();

  uint8_t Read(uint8_t addr);
  void Write(uint8_t addr, uint8_t value);

  bool enableVoice0 = true;
  bool enableVoice1 = true;
  bool enableVoice2 = true;
  bool enableVoice3 = true;
  bool enableVoice4 = true;
  bool enableVoice5 = true;
  bool enableVoice6 = true;
  bool enableVoice7 = true;

private:
  APU *memory;

  SDL_AudioStream *audioStream = nullptr;
  static const unsigned int maxSamples = 4800;
  int16_t samples[maxSamples];
  unsigned int sampleIndex = 0;

  uint8_t step = 0;
  bool evenSample = true;

  int counter = 0;

  struct voice {
    // REGISTERS
    int8_t VOLL = 0;   // Left channel volume
    int8_t VOLR = 0;   // Right channel volume
    uint16_t P = 0;    // Pitch value (14 bits)
    uint8_t SRCN = 0;  // Source for channel relative to DIR
    uint8_t ADSR1 = 0; // If bit 7 is set, enables envelope, otherwise use GAIN
    uint8_t ADSR2 = 0; // More envelope variables
    uint8_t GAIN = 0;  // Custom envelope with 5 modes
    uint8_t ENVX = 0;  // Contains the current ADSR/GAIN envelope value
    uint8_t OUTX = 0;  // Contains the present wave height multiplied by ENVX

    // Internal
    int16_t PCMBuffer[12];
    uint16_t bufferPositionInterp = 0;
    uint16_t brrAddress = 0;
    uint16_t _brrAddress = 0;
    uint8_t brrOffset = 1;

    uint8_t bufferOffset = 0;
    uint8_t brrByte = 0;
    uint8_t brrHeader = 0;

    uint8_t keyOnDelay = 0;

    uint8_t envelopeMode = 0; // 0: Release | 1: Attack | 2: Decay | 3: Sustain
    int16_t envelope = 0;

    int16_t sample = 0;
    int16_t sampleClean = 0;
  };

  voice voices[8];

  void ReadBRR(uint8_t channel);

  void ProcessSample(uint8_t channel);

  int16_t GaussInterp(uint8_t channel);

  void ProcessEnvelope(uint8_t channel);

  bool CounterPoll(unsigned int rate);

  int8_t MVOLL = 0; // Left main volume
  int8_t MVOLR = 0; // Right main volume
  int8_t EVOLL = 0; // Left echo volume
  int8_t EVOLR = 0; // Right echo volume

  uint8_t KON = 0; // Start/Restart channels
  uint8_t voicesKON = 0;
  uint8_t internalKON = 0; // Pooled every 2 samples
  uint8_t KOF = 0;         // Fade out channels

  uint8_t FLG = 0; // Bit 7: Reset. Bit 6: Mute. Bit 5: Echo enable (0 =
                   // Enable). Bit 4-0: Frequency for white noise

  uint8_t ENDX = 0; // 1 bit per channel. If set BRR decoder reached last block

  int8_t EFB = 0; // Echo feedback

  uint8_t PMON = 0; // Pitch modulation

  uint8_t NON = 0; // Noise enable
  int16_t noise = 0;
  uint8_t EON = 0; // Echo enable

  uint8_t DIR = 0; // Source directory offset

  uint8_t ESA = 0; // Echo data start address
  uint8_t EDL = 0; // (only 4 lower bits) Echo delay size
  uint8_t COEF[8]; // Echo FIR filter coefficients
};
