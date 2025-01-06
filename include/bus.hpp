#pragma once
#include <cstdint>
#include <vector>
#include <optional>

#define RAM_START 0x0000
#define RAM_END 0x1FFF
#define PPU_START 0x2000
#define PPU_END 0x3FFF

enum Mirroring {
  VERTICAL,
  HORIZONTAL,
  FOUR_SCREEN
};

struct Rom {
  uint8_t mapper;
  std::vector<uint8_t> progRom;
  std::vector<uint8_t> chrRom;
  Mirroring screenMirroring;
};

class Bus {
public:
  Bus(std::vector<uint8_t> romData);
  void writeToMemory(uint16_t address, uint8_t data);
  uint8_t readFromMemory(uint16_t address);
  void writeShortToMemory(uint16_t address, uint16_t data);
  uint16_t readShortFromMemory(uint16_t address);
  static std::optional<Rom> readBytes(std::vector<uint8_t>& raw);
private:
  uint8_t cpuVram[2048];
  uint8_t readPrgRom(uint16_t address);
  uint16_t readShortFromPrgRom(uint16_t address);
  Rom rom;
};


