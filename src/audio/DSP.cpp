#include "audio/DSP.h"

#include "audio/APU.h"

#include <cassert>
#include <stdio.h>

const uint8_t env_release = 0;
const uint8_t env_attack = 1;
const uint8_t env_decay = 2;
const uint8_t env_sustain = 3;

int16_t Clamp16(int value) {
  if ((int16_t)value != value) {
    return (value >> 31) ^ 0x7FFF;
  }
  return value;
}

static constexpr int16_t const gauss[512] = {
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
    0x000, 0x000, 0x000, 0x000, 0x000, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001,
    0x001, 0x001, 0x001, 0x001, 0x001, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002,
    0x002, 0x003, 0x003, 0x003, 0x003, 0x003, 0x004, 0x004, 0x004, 0x004, 0x004,
    0x005, 0x005, 0x005, 0x005, 0x006, 0x006, 0x006, 0x006, 0x007, 0x007, 0x007,
    0x008, 0x008, 0x008, 0x009, 0x009, 0x009, 0x00A, 0x00A, 0x00A, 0x00B, 0x00B,
    0x00B, 0x00C, 0x00C, 0x00D, 0x00D, 0x00E, 0x00E, 0x00F, 0x00F, 0x00F, 0x010,
    0x010, 0x011, 0x011, 0x012, 0x013, 0x013, 0x014, 0x014, 0x015, 0x015, 0x016,
    0x017, 0x017, 0x018, 0x018, 0x019, 0x01A, 0x01B, 0x01B, 0x01C, 0x01D, 0x01D,
    0x01E, 0x01F, 0x020, 0x020, 0x021, 0x022, 0x023, 0x024, 0x024, 0x025, 0x026,
    0x027, 0x028, 0x029, 0x02A, 0x02B, 0x02C, 0x02D, 0x02E, 0x02F, 0x030, 0x031,
    0x032, 0x033, 0x034, 0x035, 0x036, 0x037, 0x038, 0x03A, 0x03B, 0x03C, 0x03D,
    0x03E, 0x040, 0x041, 0x042, 0x043, 0x045, 0x046, 0x047, 0x049, 0x04A, 0x04C,
    0x04D, 0x04E, 0x050, 0x051, 0x053, 0x054, 0x056, 0x057, 0x059, 0x05A, 0x05C,
    0x05E, 0x05F, 0x061, 0x063, 0x064, 0x066, 0x068, 0x06A, 0x06B, 0x06D, 0x06F,
    0x071, 0x073, 0x075, 0x076, 0x078, 0x07A, 0x07C, 0x07E, 0x080, 0x082, 0x084,
    0x086, 0x089, 0x08B, 0x08D, 0x08F, 0x091, 0x093, 0x096, 0x098, 0x09A, 0x09C,
    0x09F, 0x0A1, 0x0A3, 0x0A6, 0x0A8, 0x0AB, 0x0AD, 0x0AF, 0x0B2, 0x0B4, 0x0B7,
    0x0BA, 0x0BC, 0x0BF, 0x0C1, 0x0C4, 0x0C7, 0x0C9, 0x0CC, 0x0CF, 0x0D2, 0x0D4,
    0x0D7, 0x0DA, 0x0DD, 0x0E0, 0x0E3, 0x0E6, 0x0E9, 0x0EC, 0x0EF, 0x0F2, 0x0F5,
    0x0F8, 0x0FB, 0x0FE, 0x101, 0x104, 0x107, 0x10B, 0x10E, 0x111, 0x114, 0x118,
    0x11B, 0x11E, 0x122, 0x125, 0x129, 0x12C, 0x130, 0x133, 0x137, 0x13A, 0x13E,
    0x141, 0x145, 0x148, 0x14C, 0x150, 0x153, 0x157, 0x15B, 0x15F, 0x162, 0x166,
    0x16A, 0x16E, 0x172, 0x176, 0x17A, 0x17D, 0x181, 0x185, 0x189, 0x18D, 0x191,
    0x195, 0x19A, 0x19E, 0x1A2, 0x1A6, 0x1AA, 0x1AE, 0x1B2, 0x1B7, 0x1BB, 0x1BF,
    0x1C3, 0x1C8, 0x1CC, 0x1D0, 0x1D5, 0x1D9, 0x1DD, 0x1E2, 0x1E6, 0x1EB, 0x1EF,
    0x1F3, 0x1F8, 0x1FC, 0x201, 0x205, 0x20A, 0x20F, 0x213, 0x218, 0x21C, 0x221,
    0x226, 0x22A, 0x22F, 0x233, 0x238, 0x23D, 0x241, 0x246, 0x24B, 0x250, 0x254,
    0x259, 0x25E, 0x263, 0x267, 0x26C, 0x271, 0x276, 0x27B, 0x280, 0x284, 0x289,
    0x28E, 0x293, 0x298, 0x29D, 0x2A2, 0x2A6, 0x2AB, 0x2B0, 0x2B5, 0x2BA, 0x2BF,
    0x2C4, 0x2C9, 0x2CE, 0x2D3, 0x2D8, 0x2DC, 0x2E1, 0x2E6, 0x2EB, 0x2F0, 0x2F5,
    0x2FA, 0x2FF, 0x304, 0x309, 0x30E, 0x313, 0x318, 0x31D, 0x322, 0x326, 0x32B,
    0x330, 0x335, 0x33A, 0x33F, 0x344, 0x349, 0x34E, 0x353, 0x357, 0x35C, 0x361,
    0x366, 0x36B, 0x370, 0x374, 0x379, 0x37E, 0x383, 0x388, 0x38C, 0x391, 0x396,
    0x39B, 0x39F, 0x3A4, 0x3A9, 0x3AD, 0x3B2, 0x3B7, 0x3BB, 0x3C0, 0x3C5, 0x3C9,
    0x3CE, 0x3D2, 0x3D7, 0x3DC, 0x3E0, 0x3E5, 0x3E9, 0x3ED, 0x3F2, 0x3F6, 0x3FB,
    0x3FF, 0x403, 0x408, 0x40C, 0x410, 0x415, 0x419, 0x41D, 0x421, 0x425, 0x42A,
    0x42E, 0x432, 0x436, 0x43A, 0x43E, 0x442, 0x446, 0x44A, 0x44E, 0x452, 0x455,
    0x459, 0x45D, 0x461, 0x465, 0x468, 0x46C, 0x470, 0x473, 0x477, 0x47A, 0x47E,
    0x481, 0x485, 0x488, 0x48C, 0x48F, 0x492, 0x496, 0x499, 0x49C, 0x49F, 0x4A2,
    0x4A6, 0x4A9, 0x4AC, 0x4AF, 0x4B2, 0x4B5, 0x4B7, 0x4BA, 0x4BD, 0x4C0, 0x4C3,
    0x4C5, 0x4C8, 0x4CB, 0x4CD, 0x4D0, 0x4D2, 0x4D5, 0x4D7, 0x4D9, 0x4DC, 0x4DE,
    0x4E0, 0x4E3, 0x4E5, 0x4E7, 0x4E9, 0x4EB, 0x4ED, 0x4EF, 0x4F1, 0x4F3, 0x4F5,
    0x4F6, 0x4F8, 0x4FA, 0x4FB, 0x4FD, 0x4FF, 0x500, 0x502, 0x503, 0x504, 0x506,
    0x507, 0x508, 0x50A, 0x50B, 0x50C, 0x50D, 0x50E, 0x50F, 0x510, 0x511, 0x511,
    0x512, 0x513, 0x514, 0x514, 0x515, 0x516, 0x516, 0x517, 0x517, 0x517, 0x518,
    0x518, 0x518, 0x518, 0x518, 0x519, 0x519,
};

const int CounterRange = 2048 * 5 * 3;

const uint16_t CounterRate[32] = {
    CounterRange + 1,
    2048,
    1536,
    1280,
    1024,
    768,
    640,
    512,
    384,
    320,
    256,
    192,
    160,
    128,
    96,
    80,
    64,
    48,
    40,
    32,
    24,
    20,
    16,
    12,
    10,
    8,
    6,
    5,
    4,
    3,
    2,
    1,
};

const uint16_t CounterOffset[32] = {
    1,    0,    1040, 536,  0,    1040, 536,  0,    1040, 536,  0,
    1040, 536,  0,    1040, 536,  0,    1040, 536,  0,    1040, 536,
    0,    1040, 536,  0,    1040, 536,  0,    1040, 0,    0,
};

DSP::DSP() {
  SDL_AudioSpec specs;
  specs.freq = 48000;
  specs.format = SDL_AUDIO_S16;
  specs.channels = 2;

  audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                          &specs, NULL, NULL);
  if (audioStream == NULL) {
    printf("SDL_OpenAudioDeviceStream Error: %s\n", SDL_GetError());
  }

  SDL_ResumeAudioStreamDevice(audioStream);
}

DSP::~DSP() {
  if (audioStream) {
    SDL_DestroyAudioStream(audioStream);
    audioStream = nullptr;
  }
}

bool DSP::CounterPoll(unsigned int rate) {
  return ((unsigned int)counter + CounterOffset[rate]) % CounterRate[rate];
}

void DSP::ReadBRR(uint8_t channel) {
  voice &v = voices[channel];
  int bytes = (v.brrByte << 8) | memory->GetRAM(v.brrAddress + v.brrOffset + 1);

  const uint8_t shift = (v.brrHeader >> 4) - 1;
  const uint8_t filter = (v.brrHeader >> 2) & 0x3;

  for (int i = 0; i < 4; i++) {
    int s = (int16_t)bytes >> 12;
    bytes <<= 4;

    // Apply shift
    if (shift <= 12) {
      s <<= shift;
      s >>= 1;
    } else {
      s &= ~0x7FFF;
    }

    // Apply filter
    int offset = v.bufferOffset;
    if (--offset < 0)
      offset = 11;
    const int p1 = v.PCMBuffer[offset];
    if (--offset < 0)
      offset = 11;
    const int p2 = v.PCMBuffer[offset] >> 1;

    switch (filter) {
    case 0:
      break;
    case 1:
      s += p1 >> 0;
      s += (-p1) >> 4;
      break;
    case 2:
      s += p1;
      s -= p2;
      s += p2 >> 4;
      s += (p1 * -3) >> 5;
      break;
    case 3:
      s += p1;
      s -= p2;
      s += (p1 * -13) >> 6;
      s += (p2 * 3) >> 4;
      break;
    }

    // Clamp result
    s = (int16_t)(Clamp16(s) << 1);
    v.PCMBuffer[v.bufferOffset] = s;
    if (++v.bufferOffset >= 12)
      v.bufferOffset = 0;
  }
}

int16_t DSP::GaussInterp(uint8_t channel) {
  voice &v = voices[channel];

  uint8_t position = (v.bufferPositionInterp >> 12) + v.bufferPos;
  uint8_t offset = (v.bufferPositionInterp >> 4) & 0xFF;

  int16_t olderOutput =
      (gauss[0x0FF - offset] * v.PCMBuffer[position % 12]) >> 10;
  olderOutput +=
      (gauss[0x1FF - offset] * v.PCMBuffer[(position + 1) % 12]) >> 10;
  olderOutput +=
      (gauss[0x100 + offset] * v.PCMBuffer[(position + 2) % 12]) >> 10;
  int32_t output =
      olderOutput + ((gauss[offset] * v.PCMBuffer[(position + 3) % 12]) >> 10);

  // Clamp output
  return (int16_t)(Clamp16(output) >> 1);
}

void DSP::ProcessEnvelope(uint8_t channel) {
  voice &v = voices[channel];

  int envelope = v.envelope;

  // Release
  if (v.envelopeMode == env_release) {
    envelope -= 0x8;
    if (envelope < 0)
      envelope = 0;
    v.envelope = envelope;
    return;
  }

  int rate;
  int envelopeData = v.ADSR2;
  if (v.ADSR1 & 0x80) {
    if (v.envelopeMode >= env_decay) {
      envelope -= (envelope >> 8) + 1;
      rate = envelopeData & 0x1F;
      // Decay
      if (v.envelopeMode == env_decay) {
        rate = (((v.ADSR1 >> 4) & 0x7) << 1) + 0x10;
      }
    }
    // Attack
    else {
      rate = (v.ADSR1 & 0xF) * 2 + 1;
      envelope += rate < 31 ? 0x20 : 0x400;
    }
  }
  // GAIN
  else {
    envelopeData = v.GAIN;
    int mode = envelopeData >> 5;
    if (mode < 4) { // Direct
      envelope = envelopeData * 0x10;
      rate = 31;
    } else {
      rate = envelopeData & 0x1F;
      if (mode == 4) { // Linear decrease
        envelope -= 0x20;
      } else if (mode < 6) { // Exponential decrease
        envelope--;
        envelope -= envelope >> 8;
      } else { // Linear increase
        envelope += 0x20;
        if (mode > 6 && v.envelope >= 0x600) {
          envelope += 0x8 - 0x20;
        }
      }
    }
  }

  // Sustain level
  if (envelope <= ((envelopeData >> 5) + 1) * 0x100 &&
      v.envelopeMode == env_decay) {
    v.envelopeMode = env_sustain;
  }

  if (envelope >= 0x7E0) {
    if (v.envelopeMode == env_attack) {
      v.envelopeMode = env_decay;
      if ((uint16_t)envelope > 0x7FF) {
        envelope = (envelope < 0 ? 0 : 0x7FF);
      }
    }
  }

  if (!CounterPoll(rate)) {
    v.envelope = envelope;
  }
}

void DSP::ProcessSample(uint8_t channel) {
  voice &v = voices[channel];

  // Step 1
  v._brrAddress = (DIR << 8) + (v.SRCN << 2);

  // Step 2
  uint16_t addr = v._brrAddress;
  if (!v.keyOnDelay)
    addr += 2;
  uint16_t brrNextAddress = memory->GetRAM(addr++);
  brrNextAddress |= memory->GetRAM(addr++) << 8;

  // Step 3
  uint16_t pitch = v.P;

  v.brrByte = memory->GetRAM(v.brrAddress + v.brrOffset);
  v.brrHeader = memory->GetRAM(v.brrAddress);

  if (PMON & (0x01 << channel)) {
    assert(channel != 0 && "Pitch modulation channel 0");
    pitch = (((voices[channel - 1].sample >> 4) + 0x400) * pitch) >> 10;
  }

  if (v.keyOnDelay) {
    if (v.keyOnDelay == 5) {
      v.brrAddress = brrNextAddress;
      v.brrOffset = 1;
      v.bufferOffset = 0;
      v.brrHeader = 0;
    }
    v.envelope = 0;

    v.keyOnDelay--;

    v.bufferPositionInterp = 0;
    if (v.keyOnDelay & 3)
      v.bufferPositionInterp = 0x4000;

    pitch = 0;
  }

  v.sample = GaussInterp(channel);

  if (NON & (0x01 << channel)) {
    // TODO: Noise
    v.sample = 0;
  }

  // TODO: Apply envelope
  // v.sample = (v.sample * v.envelope) / 0x800;
  v.ENVX = v.envelope >> 4;

  if ((FLG & 0x80) || (v.brrHeader & 0x01)) {
    v.envelopeMode = env_release;
    v.envelope = 0;
  }

  if (evenSample) {
    if (KOF & (0x01 << channel)) {
      v.envelopeMode = env_release;
    }

    if (voicesKON & (0x01 << channel)) {
      v.keyOnDelay = 5;
      v.envelopeMode = env_attack;
    }
  }

  if (!v.keyOnDelay) {
    ProcessEnvelope(channel);
  }

  // Step 4
  uint8_t looped = 0;
  if (v.bufferPositionInterp >= 0x4000) {
    ReadBRR(channel);
    v.brrOffset += 2;
    if (v.brrOffset >= 9) {
      v.brrAddress += 9;
      if (v.brrHeader & 0x01) {
        v.brrAddress = brrNextAddress;
        looped = 1;
      }
      v.brrOffset = 1;
    }
  }

  pitch &= 0x3FFF;
  v.bufferPositionInterp = (v.bufferPositionInterp & 0x3FFF) + pitch;
  if (v.bufferPositionInterp > 0x7FFF)
    v.bufferPositionInterp = 0x7FFF;

  // Step 5
  ENDX |= (looped << channel);
  if (v.keyOnDelay == 5) {
    ENDX &= ~(0x01 << channel);
  }

  // Step 6
  v.OUTX = v.sample >> 8;
}

void DSP::Tick() {
  // Process Voices samples
  if (step == 10) {
    ProcessSample(0);
  } else if (step == 11) {
    ProcessSample(1);
  } else if (step == 12) {
    ProcessSample(2);
  } else if (step == 13) {
    ProcessSample(3);
  } else if (step == 14) {
    ProcessSample(4);
  } else if (step == 15) {
    ProcessSample(5);
  } else if (step == 16) {
    ProcessSample(6);
  } else if (step == 17) {
    ProcessSample(7);
  } else if (step == 20) {
    evenSample = !evenSample;

    // Read KON
    if (evenSample) {
      internalKON &= !voicesKON;
      voicesKON = internalKON;
    }

    counter--;
    if (counter < 0)
      counter = CounterRange - 1;

    // TODO: Noise
  }
  // Update samples
  else if (step == 21) {
    samples[sampleIndex] = 0;
    int32_t sample = 0;
    for (int i = 0; i < 8; i++) {
      sample += (voices[i].sample * voices[i].VOLL) >> 6;
      sample = Clamp16(sample);
    }
    samples[sampleIndex] = (sample * MVOLL) >> 7;
    sampleIndex++;

    samples[sampleIndex] = 0;
    sample = 0;
    for (int i = 0; i < 8; i++) {
      sample += (voices[i].sample * voices[i].VOLR) >> 6;
      sample = Clamp16(sample);
    }
    samples[sampleIndex] = (sample * MVOLR) >> 7;
    sampleIndex++;
  }
  // Output samples
  else if (step == 31) {
    if (sampleIndex >= maxSamples) {
      sampleIndex = 0;
      while (SDL_GetAudioStreamQueued(audioStream) >
             maxSamples * sizeof(int16_t)) {
      }

      if (!SDL_PutAudioStreamData(audioStream, samples,
                                  maxSamples * sizeof(int16_t))) {
        printf("SDL_PutAudioStreamData Error: %s\n", SDL_GetError());
      }
    }
  }

  step = (step + 1) % 32;
}

uint8_t DSP::Read(uint8_t addr) {
  // Reading channels
  if ((addr & 0xF0) < 0x80 && (addr & 0x0F) < 0x0A) {
    uint8_t channel = addr >> 4;
    switch (addr & 0x0F) {
    case 0:
      return voices[channel].VOLL;
    case 1:
      return voices[channel].VOLR;
    case 2:
      return voices[channel].P & 0xFF;
    case 3:
      return voices[channel].P >> 8;
    case 4:
      return voices[channel].SRCN;
    case 5:
      return voices[channel].ADSR1;
    case 6:
      return voices[channel].ADSR2;
    case 7:
      return voices[channel].GAIN;
    case 8:
      return voices[channel].ENVX;
    case 9:
      return voices[channel].OUTX;
    }
  }
  if (addr == 0x0C)
    return MVOLL;
  if (addr == 0x1C)
    return MVOLR;
  if (addr == 0x2C)
    return EVOLL;
  if (addr == 0x3C)
    return EVOLR;
  if (addr == 0x4C)
    return KON;
  if (addr == 0x5C)
    return KOF;
  if (addr == 0x6C)
    return FLG;
  if (addr == 0x7C)
    return ENDX;
  if (addr == 0x0D)
    return EFB;
  if (addr == 0x2D)
    return PMON;
  if (addr == 0x3D)
    return NON;
  if (addr == 0x4D)
    return EON;
  if (addr == 0x5D)
    return DIR;
  if (addr == 0x6D)
    return ESA;
  if (addr == 0x7D)
    return EDL;
  if ((addr & 0xF0) < 0x80 && (addr & 0x0F) == 0x0F) {
    return COEF[addr >> 4];
  }

  printf("Reading address: %02x", addr);
  fflush(stdout);
  assert(false && "Reading address not handled");
  return 0;
}

void DSP::Write(uint8_t addr, uint8_t value) {
  // Writting channels
  if ((addr >> 4) < 0x08 && (addr & 0x0F) < 0x0A) {
    uint8_t channel = addr >> 4;
    switch (addr & 0x0F) {
    case 0:
      voices[channel].VOLL = value;
      return;
    case 1:
      voices[channel].VOLR = value;
      return;
    case 2:
      voices[channel].P = (voices[channel].P & 0xFF00) | value;
      return;
    case 3:
      voices[channel].P &= 0x00FF;
      voices[channel].P |= (value << 8);
      voices[channel].P &= 0x3FFF;
      return;
    case 4:
      voices[channel].SRCN = value;
      return;
    case 5:
      voices[channel].ADSR1 = value;
      return;
    case 6:
      voices[channel].ADSR2 = value;
      return;
    case 7:
      voices[channel].GAIN = value;
      return;
    case 8:
      voices[channel].ENVX = value;
      return;
    case 9:
      voices[channel].OUTX = value;
      return;
    }
  }
  if (addr == 0x0C) {
    MVOLL = (int8_t)value;
    return;
  } else if (addr == 0x1C) {
    MVOLR = (int8_t)value;
    return;
  } else if (addr == 0x2C) {
    EVOLL = (int8_t)value;
    return;
  } else if (addr == 0x3C) {
    EVOLR = (int8_t)value;
    return;
  } else if (addr == 0x4C) {
    KON = value;
    internalKON = value;
    return;
  } else if (addr == 0x5C) {
    KOF = value;
    return;
  } else if (addr == 0x6C) {
    FLG = value;
    return;
  } else if (addr == 0x7C) {
    ENDX = value;
    return;
  } else if (addr == 0x0D) {
    EFB = (int8_t)value;
    return;
  } else if (addr == 0x2D) {
    PMON = value;
    return;
  } else if (addr == 0x3D) {
    NON = value;
    return;
  } else if (addr == 0x4D) {
    EON = value;
    return;
  } else if (addr == 0x5D) {
    DIR = value;
    return;
  } else if (addr == 0x6D) {
    ESA = value;
    return;
  } else if (addr == 0x7D) {
    EDL = value;
    return;
  } else if ((addr & 0xF0) < 0x80 && (addr & 0x0F) == 0x0F) {
    COEF[addr >> 4] = value;
    return;
  }

  printf("Writting address %02x with value %02x\n", addr, value);
  fflush(stdout);
  assert(false && "Writting address not handled");
}
