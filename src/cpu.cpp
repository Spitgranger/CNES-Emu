#include <cstring>
#include "cpu.hpp"

CPU::CPU() {
  CPU::A = 0x00;
  CPU::X = 0x00;
  CPU::Y = 0x00;
  CPU::S = (0x00 | FLAGS::I);
  CPU::PC = 0x0000;
  CPU::SP = 0x0000;
}

CPU::~CPU() {
  // Destructor, TODO
}

uint8_t CPU::LDA(uint8_t param) {
  this->A = param;
  if (this->A == 0) {
    S |= FLAGS::Z;
  }
  if (this->A & (1 << 7)) {
    S |= FLAGS::N;
  }
  return 0;
}

uint8_t CPU::BRK() {
  this->S |= FLAGS::B;
  return 0;
}

uint8_t CPU::TAX() {
  this->X = this->A;
  if (this->X == 0) {
    S |= FLAGS::Z;
  }
  if (this->X & (1 << 7)) {
    S |= FLAGS::N;
  }
  return 0;
}

uint8_t CPU::INX() {
  this->X++;
  if (this->X == 0) {
    S |= FLAGS::Z;
  }
  if (this->X & (1 << 7)) {
    S |= FLAGS::N;
  }
  return 0;
}

uint8_t CPU::readFromMemory(uint16_t address) {
  return this->memory[address];
}

void CPU::writeToMemory(uint16_t address, uint8_t data) {
  this->memory[address] = data;
}

uint16_t CPU::readShortFromMemory(uint16_t address) {
  // 6502 uses little endian addressing
  uint16_t lo = this->memory[address];
  uint16_t hi = this->memory[address + 1];
  return (hi << 8 | lo);
}

void CPU::writeShortToMemory(uint16_t address, uint16_t data) {
  uint8_t lo = data & 0x00FF;
  uint8_t hi = data & 0xFF00;
  this->memory[address] = lo;
  this->memory[address + 1] = hi;
}

void CPU::loadProgram(uint8_t program[], uint32_t size) {
  memmove(&(this->memory[0x8000]), program, size);
  this->PC = 0x8000;
}

void CPU::loadProgramAndRun(uint8_t program[], uint32_t size) {
  loadProgram(program, size);
  interpret();
}

void CPU::interpret() {
  for (;;) {
    uint8_t opcode = this->memory[this->PC];
    this->PC++;
    switch (opcode) {
    // BRK
    case 0x00:
      BRK();
      return;
    // LDA
    case 0xA9:
      LDA(this->memory[this->PC]);
      this->PC++;
      break;
    // TAX
    case 0xAA:
      TAX();
      break;
    case 0xE8:
      INX();
      break;
    }
  }
}

void CPU::reset() {
  this->PC = readShortFromMemory(0xFFFC);
  this->S -= 3;
}

