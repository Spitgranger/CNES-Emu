#include "bus.hpp"
#include <iostream>
#include <cstring>

Bus::Bus() {
  memset(this->cpuVram, 0, sizeof(cpuVram));
}

uint8_t Bus::readFromMemory(uint16_t address) {
  if (address >= RAM_START && address <= RAM_END) {
    uint16_t mirroredAddress = address & 0b0000011111111111;
    return this->cpuVram[mirroredAddress];
  } else if (address >= PPU_START && address <= PPU_END) {
    uint16_t mirroredAddress = address & 0b0010000000000111;
    // TODO implement PPU
  }
  std::cout << "Ignoring invalid memory access at" << address << "\n";
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
  }
  std::cout << "Ignoring invalid memory access at" << address << "\n";
}

uint16_t Bus::readShortFromMemory(uint16_t address) {
  if (address >= RAM_START && address <= RAM_END) {
    uint16_t mirroredAddress = address & 0b0000011111111111;
    return ((this->cpuVram[mirroredAddress + 1]) << 8) | (this->cpuVram[mirroredAddress]);
  } else if (address >= PPU_START && address <= PPU_END) {
    uint16_t mirroredAddress = address & 0b0010000000000111;
    // TODO implement PPU
  }
  std::cout << "Ignoring invalid memory access at" << address << "\n";
  return 0;
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
  std::cout << "Ignoring invalid memory access at" << address << "\n";
}
