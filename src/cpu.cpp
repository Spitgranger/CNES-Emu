#include "cpu.hpp"
#include <cstring>
#include <iostream>

std::vector<CPU::instruction> CPU::opcodeTable = {
  {0x00, "BRK", 1, 7, ADDRESSING::NoneAddressing},
  {0xAA, "TAX", 1, 2, ADDRESSING::NoneAddressing},
  {0xE8, "INX", 1, 2, ADDRESSING::NoneAddressing},

  {0xA9, "LDA", 2, 2, ADDRESSING::Immediate},
  {0xA5, "LDA", 2, 3, ADDRESSING::ZeroPage},
  {0xB5, "LDA", 2, 4, ADDRESSING::ZeroPage_X},
  {0xAD, "LDA", 3, 4, ADDRESSING::Absolute},
  {0xBD, "LDA", 3, 4 /*+1 if page crossed*/, ADDRESSING::Absolute_X},
  {0xB9, "LDA", 3, 4 /*+1 if page crossed*/, ADDRESSING::Absolute_Y},
  {0xA1, "LDA", 2, 6, ADDRESSING::Indirect_X},
  {0xB1, "LDA", 2, 5 /*+1 if page crossed*/, ADDRESSING::Indirect_Y},

  {0x85, "STA", 2, 3, ADDRESSING::ZeroPage},
  {0x95, "STA", 2, 4, ADDRESSING::ZeroPage_X},
  {0x8D, "STA", 3, 4, ADDRESSING::Absolute},
  {0x9D, "STA", 2, 5, ADDRESSING::Absolute_X},
  {0x99, "STA", 2, 5, ADDRESSING::Absolute_Y},
  {0x81, "STA", 2, 6, ADDRESSING::Indirect_X},
  {0x91, "STA", 2, 6, ADDRESSING::Indirect_Y},
};

CPU::CPU() {
  CPU::A = 0x00;
  CPU::X = 0x00;
  CPU::Y = 0x00;
  CPU::S = (0x00 | FLAGS::I);
  CPU::PC = 0x0000;
  CPU::SP = 0x0000;

  // Best way I can think of to accomplish this for now
  for (instruction ins: CPU::opcodeTable) {
    lookupTable[ins.opcode] = ins;
  }
}

CPU::~CPU() {
  // Destructor, TODO
}

uint8_t CPU::LDA(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  uint8_t value = readFromMemory(address);
  this->A = value;
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

uint8_t CPU::STA(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  writeToMemory(address, this->A);
  return 0;
}

uint8_t CPU::readFromMemory(uint16_t address) { return this->memory[address]; }

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
    uint16_t prevProgCounter = this->PC;
    switch (opcode) {
    // BRK
    case 0x00:
      BRK();
      return;
    // LDA
    case 0xA9:
    case 0xA5:
    case 0xB5:
    case 0xAD:
    case 0xBD:
    case 0xB9:
    case 0xA1:
    case 0xB1:
      LDA(lookupTable[opcode].mode);
      break;
    // TAX
    case 0xAA:
      TAX();
      break;
    // INX
    case 0xE8:
      INX();
      break;
    // STA
    case 0x85:
    case 0x95:
    case 0x8D:
    case 0x9D:
    case 0x99:
    case 0x81:
    case 0x91:
      STA(lookupTable[opcode].mode);
      break;
    }
    if (this->PC == prevProgCounter) {
      this->PC += (lookupTable[opcode].bytes - 1);
    }
  }
}

uint16_t CPU::getOperandAddress(ADDRESSING mode) {
  switch (mode) {
  case Immediate:
    return this->PC;
  case ZeroPage:
    return readFromMemory(this->PC);
  case ZeroPage_X:
    return readFromMemory(this->PC) + this->X;
  case ZeroPage_Y:
    return readFromMemory(this->PC) + this->Y;
  case Absolute:
    return readShortFromMemory(this->PC);
  case Absolute_X:
    return readShortFromMemory(this->PC) + this->X;
  case Absolute_Y:
    return readShortFromMemory(this->PC) + this->Y;
  case Indirect_X: {
    uint8_t base = readFromMemory(this->PC);
    uint8_t pointer = base + this->X;
    uint8_t lo = readFromMemory(pointer);
    uint8_t high = readFromMemory(static_cast<uint8_t>(pointer + 1));
    return ((high << 8) | lo);
  }
  case Indirect_Y: {
    uint8_t pointer = readFromMemory(this->PC);
    uint8_t lo = readFromMemory(pointer);
    uint8_t high = readFromMemory(static_cast<uint8_t>(pointer + 1));
    return ((high << 8) | lo) + this->Y;
  }
  case NoneAddressing:
    std::cout << "Error: Addressing mode not found" << "\n";
    return -1;
  default:
    std::cout << "Error: Addressing mode not found" << "\n";
    return -1;
  }
}

void CPU::reset() {
  this->PC = readShortFromMemory(0xFFFC);
  this->S -= 3;
}
