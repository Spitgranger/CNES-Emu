#include "bus.hpp"
#include <cstring>
#include <iostream>

#define PRG_ROM_PAGE_SIZE 0x4000
#define CHR_ROM_PAGE_SIZE 0x2000

Bus::Bus(std::vector<uint8_t> romData) { 
  memset(this->cpuVram, 0, sizeof(cpuVram));
  std::optional<Rom> decodedRom = readBytes(romData);
  if (decodedRom.has_value()) {
    this->rom = decodedRom.value();
  } else {
    this->rom = Rom{};
  }
}

uint8_t Bus::readFromMemory(uint16_t address) {
  if (address >= RAM_START && address <= RAM_END) {
    uint16_t mirroredAddress = address & 0b0000011111111111;
    return this->cpuVram[mirroredAddress];
  } else if (address >= 0x8000 && address <= 0xFFFF) {
    return readPrgRom(address);
  }
  else if (address >= PPU_START && address <= PPU_END) {
    uint16_t mirroredAddress = address & 0b0010000000000111;
    // TODO implement PPU
  }
  std::cout << "Read From Memory: Ignoring invalid memory access at " << address << "\n";
  return 0;
}

void Bus::writeToMemory(uint16_t address, uint8_t data) {
  if (address >= RAM_START && address <= RAM_END) {
    uint16_t mirroredAddress = address & 0b11111111111;
    this->cpuVram[mirroredAddress] = data;
    return;
  } else if (address >= PPU_START && address <= PPU_END) {
    uint16_t mirroredAddress = address & 0b0010000000000111;
    // TODO implement PPU
    return;
  } else if (address >= 0x8000 && address <= 0xFFFF) {
    std::cout << "Attempt write to prog rom at " << address << "\n";
  }
  std::cout << " Write to memory: Ignoring invalid memory access at " << address << "\n";
}

uint16_t Bus::readShortFromMemory(uint16_t address) {
  if (address >= RAM_START && address <= RAM_END) {
    uint16_t mirroredAddress = address & 0b0000011111111111;
    return ((this->cpuVram[mirroredAddress + 1]) << 8) |
           (this->cpuVram[mirroredAddress]);
  } else if (address >= PPU_START && address <= PPU_END) {
    uint16_t mirroredAddress = address & 0b0010000000000111;
    // TODO implement PPU
  } else if (address >= 0x8000 && address <= 0xFFFF) {
    return readShortFromPrgRom(address);
  }
  std::cout << "Read Short From Memory: Ignoring invalid memory access at " << address << "\n";
  return 0;
}

uint8_t Bus::readPrgRom(uint16_t address) {
  address -= 0x8000;
  if (this->rom.progRom.size() == 0x4000 && address >= 0x4000) {
    address = address % 0x4000;
  }
  return this->rom.progRom[address];
}

uint16_t Bus::readShortFromPrgRom(uint16_t address) {
  address -= 0x8000;
  if (this->rom.progRom.size() == 0x4000 && address >= 0x4000) {
    address = address % 0x4000;
  }
  uint16_t lo = this->rom.progRom[address];
  uint16_t hi = this->rom.progRom[address + 1];
  return (hi << 8) | lo;
}

void Bus::writeShortToMemory(uint16_t address, uint16_t data) {
  if (address >= RAM_START && address <= RAM_END) {
    uint16_t mirroredAddress = address & 0b11111111111;
    this->cpuVram[mirroredAddress] = data & 0xFF;
    this->cpuVram[mirroredAddress + 1] = data >> 8;
    return;
  } else if (address >= PPU_START && address <= PPU_END) {
    uint16_t mirroredAddress = address & 0b0010000000000111;
    // TODO implement PPU
    return;
  }
  std::cout << "Write Short to memory: Ignoring invalid memory access at" << address << "\n";
}

std::optional<Rom> Bus::readBytes(std::vector<uint8_t>& raw) {
  if (raw.empty()) {
    // Method for testing, allows for construction of a bus with an empty ROM
    return {};
  }
  if (raw[0] != 0x4E || raw[1] != 0x45 || raw[2] != 0x53 || raw[3] != 0x1A) {
    return {};
  }
  uint8_t mapper = (raw[7] & 0b11110000) | (raw[6] >> 4);
  uint8_t inesVersion = (raw[7] >> 2) & 0b11;
  if (inesVersion != 0) {
    return {};
  }
  bool fourScreen = (raw[6] & 0b1000) != 0;
  bool verticalMirroring = (raw[6] & 0b1) != 0;
  Mirroring screenMirroring;
  if (fourScreen) {
    screenMirroring = FOUR_SCREEN;
  } else if (!fourScreen && verticalMirroring) {
    screenMirroring = VERTICAL;
  } else if (!fourScreen && !verticalMirroring) {
    screenMirroring = HORIZONTAL;
  }

  bool skipTrainer = (raw[6] & 0b100) != 0;
  uint32_t prgRomSize = raw[4] * PRG_ROM_PAGE_SIZE;
  uint32_t chrRomSize = raw[5] * CHR_ROM_PAGE_SIZE;

  uint32_t prgRomStart = 16 + 512 * skipTrainer;
  uint32_t chrRomStart = prgRomStart + prgRomSize;
  return Rom{mapper,
          std::vector<uint8_t>(raw.begin() + prgRomStart,
                               raw.begin() + prgRomStart + prgRomSize),
          std::vector<uint8_t>(raw.begin() + chrRomStart, raw.begin() + chrRomStart + chrRomSize), screenMirroring};
}
