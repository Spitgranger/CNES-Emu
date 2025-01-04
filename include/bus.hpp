#pragma once
#include <cstdint>

#define RAM_START 0x0000
#define RAM_END 0x1FFF
#define PPU_START 0x2000
#define PPU_END 0x3FFF

class Bus {
public:
  Bus();
  void writeToMemory(uint16_t address, uint8_t data);
  uint8_t readFromMemory(uint16_t address);
  void writeShortToMemory(uint16_t address, uint16_t data);
  uint16_t readShortFromMemory(uint16_t address);
private:
  uint8_t cpuVram[2048];
};

