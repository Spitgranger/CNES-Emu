#include "cpu.hpp"
#include <cstring>
#include <iostream>

#define TOP_OF_STACK 0xFF

std::vector<CPU::instruction> CPU::opcodeTable = {
    {0x00, "BRK", 1, 7, ADDRESSING::NoneAddressing},
    {0xAA, "TAX", 1, 2, ADDRESSING::NoneAddressing},
    {0xE8, "INX", 1, 2, ADDRESSING::NoneAddressing},
    {0x90, "BCC", 2, 2 /*+1 if branch succeeds, +2 if to a new page*/,
     ADDRESSING::NoneAddressing},
    {0xB0, "BCS", 2, 2 /*+1 if branch succeeds, +2 if to a new page*/,
     ADDRESSING::NoneAddressing},
    {0xF0, "BEQ", 2, 2 /*+1 if branch succeeds, +2 if to a new page*/,
     ADDRESSING::NoneAddressing},
    {0x30, "BMI", 2, 2 /*+1 if branch succeeds, +2 if to a new page*/,
     ADDRESSING::NoneAddressing},
    {0xD0, "BNE", 2, 2 /*+1 if branch succeeds, +2 if to a new page*/,
     ADDRESSING::NoneAddressing},
    {0x10, "BPL", 2, 2 /*+1 if branch succeeds, +2 if to a new page*/,
     ADDRESSING::NoneAddressing},
    {0x50, "BVC", 2, 2 /*+1 if branch succeeds, +2 if to a new page*/,
     ADDRESSING::NoneAddressing},
    {0x70, "BVS", 2, 2 /*+1 if branch succeeds, +2 if to a new page*/,
     ADDRESSING::NoneAddressing},
    {0x18, "CLC", 1, 2, ADDRESSING::NoneAddressing},
    {0xD8, "CLD", 1, 2, ADDRESSING::NoneAddressing},
    {0x58, "CLI", 1, 2, ADDRESSING::NoneAddressing},
    {0xB8, "CLV", 1, 2, ADDRESSING::NoneAddressing},

    {0x24, "BIT", 2, 3, ADDRESSING::ZeroPage},
    {0x2C, "BIT", 3, 4, ADDRESSING::Absolute},

    {0x69, "ADC", 2, 2, ADDRESSING::Immediate},
    {0x65, "ADC", 2, 3, ADDRESSING::ZeroPage},
    {0x75, "ADC", 2, 4, ADDRESSING::ZeroPage_X},
    {0x6D, "ADC", 3, 4, ADDRESSING::Absolute},
    {0x7D, "ADC", 3, 4 /*+1 if page crossed*/, ADDRESSING::Absolute_X},
    {0x79, "ADC", 3, 4 /*+1 if page crossed*/, ADDRESSING::Absolute_Y},
    {0x61, "ADC", 2, 6, ADDRESSING::Indirect_X},
    {0x71, "ADC", 2, 5 /*+1 if page crossed*/, ADDRESSING::Indirect_Y},

    {0x29, "AND", 2, 2, ADDRESSING::Immediate},
    {0x25, "AND", 2, 3, ADDRESSING::ZeroPage},
    {0x35, "AND", 2, 4, ADDRESSING::ZeroPage_X},
    {0x2D, "AND", 3, 4, ADDRESSING::Absolute},
    {0x3D, "AND", 3, 4 /*+1 if page crossed*/, ADDRESSING::Absolute_X},
    {0x39, "AND", 3, 4 /*+1 if page crossed*/, ADDRESSING::Absolute_Y},
    {0x21, "AND", 2, 6, ADDRESSING::Indirect_X},
    {0x31, "AND", 2, 5 /*+1 if page crossed*/, ADDRESSING::Indirect_Y},

    {0xC9, "CMP", 2, 2, ADDRESSING::Immediate},
    {0xC5, "CMP", 2, 3, ADDRESSING::ZeroPage},
    {0xD5, "CMP", 2, 4, ADDRESSING::ZeroPage_X},
    {0xCD, "CMP", 3, 4, ADDRESSING::Absolute},
    {0xDD, "CMP", 3, 4 /*+1 if page crossed*/, ADDRESSING::Absolute_X},
    {0xD9, "CMP", 3, 4 /*+1 if page crossed*/, ADDRESSING::Absolute_Y},
    {0xC1, "CMP", 2, 6, ADDRESSING::Indirect_X},
    {0xD1, "CMP", 2, 5 /*+1 if page crossed*/, ADDRESSING::Indirect_Y},

    {0xE0, "CPX", 2, 2, ADDRESSING::Immediate},
    {0xE4, "CPX", 2, 3, ADDRESSING::ZeroPage},
    {0xEC, "CPX", 3, 4, ADDRESSING::Absolute},

    {0xC0, "CPY", 2, 2, ADDRESSING::Immediate},
    {0xC4, "CPY", 2, 3, ADDRESSING::ZeroPage},
    {0xCC, "CPY", 3, 4, ADDRESSING::Absolute},

    {0xC6, "DEC", 2, 5, ADDRESSING::ZeroPage},
    {0xD6, "DEC", 2, 6, ADDRESSING::ZeroPage_X},
    {0xCE, "DEC", 3, 6, ADDRESSING::Absolute},
    {0xDE, "DEC", 3, 7, ADDRESSING::Absolute_X},

    {0xCA, "DEX", 1, 2, ADDRESSING::NoneAddressing},
    {0x88, "DEY", 1, 2, ADDRESSING::NoneAddressing},

    {0x49, "EOR", 2, 2, ADDRESSING::Immediate},
    {0x45, "EOR", 2, 3, ADDRESSING::ZeroPage},
    {0x55, "EOR", 2, 4, ADDRESSING::ZeroPage_X},
    {0x4D, "EOR", 3, 4, ADDRESSING::Absolute},
    {0x5D, "EOR", 3, 4 /*+1 if page crossed*/, ADDRESSING::Absolute_X},
    {0x59, "EOR", 3, 4 /*+1 if page crossed*/, ADDRESSING::Absolute_Y},
    {0x41, "EOR", 2, 6, ADDRESSING::Indirect_X},
    {0x51, "EOR", 2, 5 /*+1 if page crossed*/, ADDRESSING::Indirect_Y},

    {0xE6, "INC", 2, 5, ADDRESSING::ZeroPage},
    {0xF6, "INC", 2, 6, ADDRESSING::ZeroPage_X},
    {0xEE, "INC", 3, 6, ADDRESSING::Absolute},
    {0xFE, "INC", 3, 7, ADDRESSING::Absolute_X},

    {0xC8, "INY", 1, 2, ADDRESSING::NoneAddressing},

    {0x4C, "JMP", 3, 3, ADDRESSING::Absolute},
    {0x6C, "JMP", 3, 5, ADDRESSING::Indirect},

    {0x20, "JSR", 3, 6, ADDRESSING::Absolute},

    {0xA9, "LDA", 2, 2, ADDRESSING::Immediate},
    {0xA5, "LDA", 2, 3, ADDRESSING::ZeroPage},
    {0xB5, "LDA", 2, 4, ADDRESSING::ZeroPage_X},
    {0xAD, "LDA", 3, 4, ADDRESSING::Absolute},
    {0xBD, "LDA", 3, 4 /*+1 if page crossed*/, ADDRESSING::Absolute_X},
    {0xB9, "LDA", 3, 4 /*+1 if page crossed*/, ADDRESSING::Absolute_Y},
    {0xA1, "LDA", 2, 6, ADDRESSING::Indirect_X},
    {0xB1, "LDA", 2, 5 /*+1 if page crossed*/, ADDRESSING::Indirect_Y},

    {0xA2, "LDX", 2, 2, ADDRESSING::Immediate},
    {0xA6, "LDX", 2, 3, ADDRESSING::ZeroPage},
    {0xB6, "LDX", 2, 4, ADDRESSING::ZeroPage_Y},
    {0xAE, "LDX", 3, 4, ADDRESSING::Absolute},
    {0xBE, "LDX", 3, 4 /*+1 if page crossed*/, ADDRESSING::Absolute_Y},

    {0xA0, "LDY", 2, 2, ADDRESSING::Immediate},
    {0xA4, "LDY", 2, 3, ADDRESSING::ZeroPage},
    {0xB4, "LDY", 2, 4, ADDRESSING::ZeroPage_X},
    {0xAC, "LDY", 3, 4, ADDRESSING::Absolute},
    {0xBC, "LDY", 3, 4 /*+1 if page crossed*/, ADDRESSING::Absolute_X},

    {0x4A, "LSR", 1, 2, ADDRESSING::NoneAddressing},
    {0x46, "LSR", 2, 5, ADDRESSING::ZeroPage},
    {0x56, "LSR", 2, 6, ADDRESSING::ZeroPage_X},
    {0x4E, "LSR", 3, 6, ADDRESSING::Absolute},
    {0x5E, "LSR", 3, 7, ADDRESSING::Absolute_X},

    {0xEA, "NOP", 1, 2, ADDRESSING::NoneAddressing},

    {0x09, "ORA", 2, 2, ADDRESSING::Immediate},
    {0x05, "ORA", 2, 3, ADDRESSING::ZeroPage},
    {0x15, "ORA", 2, 4, ADDRESSING::ZeroPage_X},
    {0x0D, "ORA", 3, 4, ADDRESSING::Absolute},
    {0x1D, "ORA", 3, 4 /*+1 if page crossed*/, ADDRESSING::Absolute_X},
    {0x19, "ORA", 3, 4 /*+1 if page crossed*/, ADDRESSING::Absolute_Y},
    {0x01, "ORA", 2, 6, ADDRESSING::Indirect_X},
    {0x11, "ORA", 2, 5 /*+1 if page crossed*/, ADDRESSING::Indirect_Y},

    {0x48, "PHA", 1, 3, ADDRESSING::NoneAddressing},
    {0x08, "PHP", 1, 3, ADDRESSING::NoneAddressing},
    {0x08, "PLA", 1, 3, ADDRESSING::NoneAddressing},
    {0x08, "PLP", 1, 3, ADDRESSING::NoneAddressing},

    {0x2A, "ROL", 1, 2, ADDRESSING::NoneAddressing},
    {0x26, "ROL", 2, 5, ADDRESSING::ZeroPage},
    {0x36, "ROL", 2, 6, ADDRESSING::ZeroPage_X},
    {0x2E, "ROL", 3, 6, ADDRESSING::Absolute},
    {0x3E, "ROL", 3, 7, ADDRESSING::Absolute_X},

    {0x6A, "ROR", 1, 2, ADDRESSING::NoneAddressing},
    {0x66, "ROR", 2, 5, ADDRESSING::ZeroPage},
    {0x76, "ROR", 2, 6, ADDRESSING::ZeroPage_X},
    {0x6E, "ROR", 3, 6, ADDRESSING::Absolute},
    {0x7E, "ROR", 3, 7, ADDRESSING::Absolute_X},

    {0xE9, "SBC", 2, 2, ADDRESSING::Immediate},
    {0xE5, "SBC", 2, 3, ADDRESSING::ZeroPage},
    {0xF5, "SBC", 2, 4, ADDRESSING::ZeroPage_X},
    {0xED, "SBC", 3, 4, ADDRESSING::Absolute},
    {0xFD, "SBC", 3, 4 /*+1 if page crossed*/, ADDRESSING::Absolute_X},
    {0xF9, "SBC", 3, 4 /*+1 if page crossed*/, ADDRESSING::Absolute_Y},
    {0xE1, "SBC", 2, 6, ADDRESSING::Indirect_X},
    {0xF1, "SBC", 2, 5 /*+1 if page crossed*/, ADDRESSING::Indirect_Y},

    {0x40, "RTI", 1, 6, ADDRESSING::NoneAddressing},
    {0x60, "RTS", 1, 6, ADDRESSING::NoneAddressing},

    {0x38, "SEC", 1, 6, ADDRESSING::NoneAddressing},
    {0xF8, "SED", 1, 6, ADDRESSING::NoneAddressing},
    {0x78, "SEI", 1, 6, ADDRESSING::NoneAddressing},

    {0x86, "STX", 2, 3, ADDRESSING::ZeroPage},
    {0x96, "STX", 2, 4, ADDRESSING::ZeroPage_Y},
    {0x8E, "STX", 3, 4, ADDRESSING::Absolute},

    {0x84, "STY", 2, 3, ADDRESSING::ZeroPage},
    {0x94, "STY", 2, 4, ADDRESSING::ZeroPage_X},
    {0x8C, "STY", 3, 4, ADDRESSING::Absolute},

    {0xA8, "TAY", 1, 2, ADDRESSING::NoneAddressing},
    {0xBA, "TSX", 1, 2, ADDRESSING::NoneAddressing},
    {0x8A, "TXA", 1, 2, ADDRESSING::NoneAddressing},
    {0x9A, "TXS", 1, 2, ADDRESSING::NoneAddressing},
    {0x98, "TYA", 1, 2, ADDRESSING::NoneAddressing},

    {0x0A, "ASL", 1, 2, ADDRESSING::NoneAddressing},
    {0x06, "ASL", 2, 5, ADDRESSING::ZeroPage},
    {0x16, "ASL", 2, 6, ADDRESSING::ZeroPage_X},
    {0x0E, "ASL", 3, 6, ADDRESSING::Absolute},
    {0x1E, "ASL", 3, 7, ADDRESSING::Absolute_X},

    {0x85, "STA", 2, 3, ADDRESSING::ZeroPage},
    {0x95, "STA", 2, 4, ADDRESSING::ZeroPage_X},
    {0x8D, "STA", 3, 4, ADDRESSING::Absolute},
    {0x9D, "STA", 2, 5, ADDRESSING::Absolute_X},
    {0x99, "STA", 2, 5, ADDRESSING::Absolute_Y},
    {0x81, "STA", 2, 6, ADDRESSING::Indirect_X},
    {0x91, "STA", 2, 6, ADDRESSING::Indirect_Y},
};

CPU::CPU() {
  this->A = 0x00;
  this->X = 0x00;
  this->Y = 0x00;
  this->S = (0x00 | FLAGS::I);
  this->PC = 0x0000;
  this->SP = TOP_OF_STACK;

  // Best way I can think of to accomplish this for now
  for (instruction ins : CPU::opcodeTable) {
    lookupTable[ins.opcode] = ins;
  }
}

CPU::~CPU() {
  // Destructor, TODO
}

void CPU::setZeroAndNegativeFlags(uint8_t value) {
  if (value == 0) {
    S |= FLAGS::Z;
  } else {
    S &= ~(FLAGS::Z);
  }
  if (value & (1 << 7)) {
    S |= FLAGS::N;
  } else {
    S &= ~(FLAGS::N);
  }
}

uint8_t CPU::LDA(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  uint8_t value = readFromMemory(address);
  this->A = value;
  setZeroAndNegativeFlags(this->A);
  return 0;
}

uint8_t CPU::LDX(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  uint8_t value = readFromMemory(address);
  this->X = value;
  setZeroAndNegativeFlags(this->X);
  return 0;
}

uint8_t CPU::LDY(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  uint8_t value = readFromMemory(address);
  this->Y = value;
  setZeroAndNegativeFlags(this->Y);
  return 0;
}

uint8_t CPU::BRK() {
  this->S |= FLAGS::B;
  return 0;
}

uint8_t CPU::TAX() {
  this->X = this->A;
  setZeroAndNegativeFlags(this->X);
  return 0;
}

uint8_t CPU::INX() {
  this->X++;
  setZeroAndNegativeFlags(this->X);
  return 0;
}

uint8_t CPU::STA(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  writeToMemory(address, this->A);
  return 0;
}

uint8_t CPU::ADC(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  uint8_t data = readFromMemory(address);
  uint16_t sum = this->A + data + (this->S & FLAGS::C);

  if (sum > 0xFF) {
    this->S |= FLAGS::C;
  } else {
    this->S &= ~(FLAGS::C);
  }

  uint8_t result = static_cast<uint8_t>(sum);

  // Check if overflow occurs as a result of signed addition.
  // if the accumulator and data have different signs, not possible for
  // overflow, but if they have the same signs and the result and accumulator
  // has a different sign, we know that overflow has occurred.
  if (~(this->A ^ data) & (this->A ^ result) & (1 << 7)) {
    this->S |= FLAGS::V;
  } else {
    this->S &= ~(FLAGS::V);
  }

  this->A = result;
  setZeroAndNegativeFlags(this->A);
  return 0;
}

uint8_t CPU::AND(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  uint8_t operand = readFromMemory(address);
  this->A &= operand;
  setZeroAndNegativeFlags(this->A);
  return 0;
}

uint8_t CPU::ASL(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  uint8_t operand = readFromMemory(address);
  if (operand >> 7) {
    this->S |= FLAGS::C;
  } else {
    this->S &= ~(FLAGS::C);
  }
  operand <<= 1;
  writeToMemory(address, operand);
  setZeroAndNegativeFlags(operand);
  return 0;
}

uint8_t CPU::ASLAccumulator() {
  if (this->A >> 7) {
    this->S |= FLAGS::C;
  } else {
    this->S &= ~(FLAGS::C);
  }
  this->A <<= 1;
  setZeroAndNegativeFlags(this->A);
  return 0;
}

void CPU::branch(bool condition) {
  if (condition) {
    int8_t offset = static_cast<int8_t>(readFromMemory(this->PC));
    uint16_t newAddress = this->PC + 1 + static_cast<uint16_t>(offset);
    this->PC = newAddress & 0xFFFF;
  }
}

uint8_t CPU::BIT(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  uint8_t operand = readFromMemory(address);
  uint8_t result = this->A & operand;
  if (result == 0) {
    this->S |= FLAGS::Z;
  } else {
    this->S &= ~(FLAGS::Z);
  }

  if (operand >> 7) {
    this->S |= FLAGS::N;
  } else {
    this->S &= ~(FLAGS::N);
  }

  if (operand >> 6) {
    this->S |= FLAGS::V;
  } else {
    this->S &= ~(FLAGS::V);
  }

  return 0;
}

void CPU::compare(ADDRESSING mode, uint8_t reg) {
  uint16_t address = getOperandAddress(mode);
  uint8_t data = readFromMemory(address);
  if (reg >= data) {
    this->S |= FLAGS::C;
  } else {
    this->S &= ~(FLAGS::C);
  }
  setZeroAndNegativeFlags(static_cast<uint8_t>(reg - data));
}

uint8_t CPU::DEC(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  uint8_t data = readFromMemory(address);

  data -= 1;
  setZeroAndNegativeFlags(data);
  writeToMemory(address, data);
  return 0;
}

void CPU::DECX() {
  uint8_t data = this->X;

  data -= 1;
  setZeroAndNegativeFlags(data);
  this->X = data;
}

void CPU::DECY() {
  uint8_t data = this->Y;

  data -= 1;
  setZeroAndNegativeFlags(data);
  this->Y = data;
}

uint8_t CPU::EOR(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  uint8_t data = readFromMemory(address);
  this->A ^= data;
  setZeroAndNegativeFlags(this->A);
  return 0;
}

uint8_t CPU::INC(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  uint8_t data = readFromMemory(address);
  data += 1;
  setZeroAndNegativeFlags(data);
  writeToMemory(address, data);
  return 0;
}

void CPU::INCY() {
  this->Y++;
  setZeroAndNegativeFlags(this->Y);
}

uint8_t CPU::JMP(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  this->PC = address;
  return 0;
}

uint8_t CPU::JSR(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  uint16_t stackValue = this->PC + 1;
  pushOnStack(static_cast<uint8_t>(stackValue >> 8));
  pushOnStack(static_cast<uint8_t>(stackValue & 0xFF));
  this->PC = address;
  return 0;
}

uint8_t CPU::LSRAccumulator() {
  if (this->A & 0x1) {
    this->S |= FLAGS::C;
  } else {
    this->S &= ~(FLAGS::C);
  }
  this->A >>= 1;
  setZeroAndNegativeFlags(this->A);
  return 0;
}

uint8_t CPU::LSR(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  uint8_t data = readFromMemory(address);
  if (data & 0x1) {
    this->S |= FLAGS::C;
  } else {
    this->S &= ~(FLAGS::C);
  }
  data >>= 1;
  setZeroAndNegativeFlags(data);
  writeToMemory(address, data);
  return 0;
}

uint8_t CPU::ORA(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  uint8_t data = readFromMemory(address);
  this->A |= data;
  setZeroAndNegativeFlags(this->A);
  return 0;
}

uint8_t CPU::PHA() {
  pushOnStack(this->A);
  return 0;
}

uint8_t CPU::PHP() {
  uint8_t s = this->S;
  s |= FLAGS::B;
  s |= FLAGS::U;
  pushOnStack(s | FLAGS::B);
  return 0;
}

uint8_t CPU::PLA() {
  uint8_t newA = popFromStack();
  this->A = newA;
  setZeroAndNegativeFlags(this->A);
  return 0;
}

uint8_t CPU::PLP() {
  uint8_t newStatus = popFromStack();
  this->S = newStatus;
  this->S &= (~FLAGS::B);
  this->S |= FLAGS::U;
  return 0;
}

uint8_t CPU::ROLAccumulator() {
  // Set Carry flag to 7th bit of the value and move carry bit into bit 0
  uint8_t c = (this->S & FLAGS::C);
  this->S |= ((this->A & (1 << 7)) >> 7);
  this->A <<= 1;
  this->A |= c;
  setZeroAndNegativeFlags(this->A);
  return 0;
}

uint8_t CPU::ROL(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  uint8_t data = readFromMemory(address);
  // Set Carry flag to 7th bit of the value and move carry bit into bit 0
  uint8_t c = (this->S & FLAGS::C);
  this->S |= ((data & (1 << 7)) >> 7);
  data <<= 1;
  data |= c;
  setZeroAndNegativeFlags(data);
  writeToMemory(address, data);
  return 0;
}

uint8_t CPU::RORAccumulator() {
  // Set Carry flag to 0th bit of the value and move carry bit into bit 7
  uint8_t c = (this->S & FLAGS::C);
  this->S |= (this->A & (1 << 0));
  this->A >>= 1;
  this->A |= (c << 7);
  setZeroAndNegativeFlags(this->A);
  return 0;
}

uint8_t CPU::ROR(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  uint8_t data = readFromMemory(address);
  // Set Carry flag to 7th bit of the value and move carry bit into bit 7
  uint8_t c = (this->S & FLAGS::C);
  this->S |= (data & (1 << 0));
  data >>= 1;
  data |= (c << 7);
  setZeroAndNegativeFlags(data);
  writeToMemory(address, data);
  return 0;
}

uint8_t CPU::RTI() {
  // TODO VERIFY
  uint8_t status = popFromStack();
  status &= (~FLAGS::B);
  status |= (~FLAGS::U);
  uint16_t lo = popFromStack();
  uint16_t hi = popFromStack();
  this->S = status;
  this->PC = ((hi << 8) | lo);
  return 0;
}

uint8_t CPU::RTS() {
  // TODO VERIFY
  uint16_t lo = popFromStack();
  uint16_t hi = popFromStack();
  this->PC = (((hi << 8) | lo) + 1);
  return 0;
}

uint8_t CPU::SBC(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  uint8_t data = readFromMemory(address);

  // Calculate the effective carry: 1 if carry flag is set, 0 otherwise
  uint8_t carry = (this->S & FLAGS::C) ? 0 : 1;

  // Perform subtraction using two's complement arithmetic
  uint16_t result = this->A - data - carry;

  // Update Carry Flag (set if result >= 0)
  if (result <= 0xFF) {
    this->S |= FLAGS::C;
  } else {
    this->S &= ~FLAGS::C;
  }

  // Update Overflow Flag (set if signed overflow occurs)
  if (((this->A ^ data) & 0x80) && ((this->A ^ result) & 0x80)) {
    this->S |= FLAGS::V;
  } else {
    this->S &= ~FLAGS::V;
  }

  // Update Accumulator and Flags
  this->A = static_cast<uint8_t>(result);
  setZeroAndNegativeFlags(this->A);

//  uint16_t sum = this->A + (~data) + (this->S & FLAGS::C);
//
//  if (~(sum < 0x00)) {
//    this->S |= FLAGS::C;
//  } else {
//    this->S &= ~(FLAGS::C);
//  }
//
//  uint8_t result = static_cast<uint8_t>(sum);
//
//  // Check if overflow occurs as a result of signed addition.
//  // if the accumulator and data have different signs, not possible for
//  // overflow, but if they have the same signs and the result and accumulator
//  // has a different sign, we know that overflow has occurred.
//  if ((this->A ^ result) & (result ^ (~data)) & (1 << 7)) {
//    this->S |= FLAGS::V;
//  } else {
//    this->S &= ~(FLAGS::V);
//  }
//
//  this->A = result;
//  setZeroAndNegativeFlags(this->A);
  return 0;
}

uint8_t CPU::SEC() {
  this->S |= FLAGS::C;
  return 0;
}

uint8_t CPU::SED() {
  this->S |= FLAGS::D;
  return 0;
}

uint8_t CPU::SEI() {
  this->S |= FLAGS::I;
  return 0;
}

uint8_t CPU::STX(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  writeToMemory(address, this->X);
  return 0;
}

uint8_t CPU::STY(ADDRESSING mode) {
  uint16_t address = getOperandAddress(mode);
  writeToMemory(address, this->Y);
  return 0;
}

uint8_t CPU::TAY() {
  this->Y = this->A;
  setZeroAndNegativeFlags(this->Y);
  return 0;
}

uint8_t CPU::TSX() {
  this->X = this->SP;
  setZeroAndNegativeFlags(this->X);
  return 0;
}

uint8_t CPU::TXA() {
  this->A = this->X;
  setZeroAndNegativeFlags(this->A);
  return 0;
}

uint8_t CPU::TXS() {
  this->SP = this->X;
  return 0;
}

uint8_t CPU::TYA() {
  this->A = this->Y;
  setZeroAndNegativeFlags(this->A);
  return 0;
}

uint8_t CPU::readFromMemory(uint16_t address) { return this->memory[address]; }

void CPU::writeToMemory(uint16_t address, uint8_t data) {
  //std::cout << std::hex << address << "\n";
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
  uint8_t hi = ((data & 0xFF00) >> 8);
  this->memory[address] = lo;
  this->memory[address + 1] = hi;
}

void CPU::loadProgram(uint8_t program[], uint32_t size) {
  memmove(&(this->memory[0x0600]), program, size);
  writeShortToMemory(0xFFFC, 0x0600);
}

void CPU::loadProgramAndRun(uint8_t program[], uint32_t size) {
  loadProgram(program, size);
  reset();
  interpret();
}

void CPU::pushOnStack(uint8_t value) {
  writeToMemory(static_cast<uint16_t>((0x0100) + (this->SP)), value);
  this->SP--;
}

uint8_t CPU::popFromStack() {
  this->SP++;
  uint8_t value = readFromMemory(((0x0100) + this->SP));
  return value;
}

void CPU::interpret() { interpretWithCB(nullptr); }

void CPU::interpretWithCB(const std::function<void(CPU *)> &callback) {
  for (;;) {
    if (callback != nullptr) {
      callback(this);
    }
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
    // ADC
    case 0x69:
    case 0x65:
    case 0x75:
    case 0x6D:
    case 0x7D:
    case 0x79:
    case 0x61:
    case 0x71:
      ADC(lookupTable[opcode].mode);
      break;
    // AND
    case 0x29:
    case 0x25:
    case 0x35:
    case 0x2D:
    case 0x3D:
    case 0x39:
    case 0x21:
    case 0x31:
      AND(lookupTable[opcode].mode);
      break;
    // ASL
    case 0x0A:
      ASLAccumulator();
      break;
    case 0x06:
    case 0x16:
    case 0x0E:
    case 0x1E:
      ASL(lookupTable[opcode].mode);
      break;
    // BCC
    case 0x90:
      branch(!(this->S & FLAGS::C));
      break;
    // BCS
    case 0xB0:
      branch((this->S & FLAGS::C));
      break;
    // BEQ
    case 0xF0:
      branch((this->S & FLAGS::Z));
      break;
    // BIT
    case 0x24:
    case 0x2C:
      BIT(lookupTable[opcode].mode);
      break;
    // BMI
    case 0x30:
      branch((this->S & FLAGS::N));
      break;
    // BNE
    case 0xD0:
      branch(!(this->S & FLAGS::Z));
      break;
    // BPL
    case 0x10:
      branch(!(this->S & FLAGS::N));
      break;
    // BVC
    case 0x50:
      branch(!(this->S & FLAGS::V));
      break;
    // BVS
    case 0x70:
      branch((this->S & FLAGS::V));
      break;
    // CLC
    case 0x18:
      this->S &= (~FLAGS::C);
      break;
    // CLD
    case 0xD8:
      this->S &= (~FLAGS::D);
      break;
    // CLI
    case 0x58:
      this->S &= (~FLAGS::I);
      break;
    // CLV
    case 0xB8:
      this->S &= (~FLAGS::V);
      break;
    // CMP
    case 0xC9:
    case 0xC5:
    case 0xD5:
    case 0xCD:
    case 0xDD:
    case 0xD9:
    case 0xC1:
    case 0xD1:
      compare(lookupTable[opcode].mode, this->A);
      break;
    // CPX
    case 0xE0:
    case 0xE4:
    case 0xEC:
      compare(lookupTable[opcode].mode, this->X);
      break;
    // CPY
    case 0xC0:
    case 0xC4:
    case 0xCC:
      compare(lookupTable[opcode].mode, this->Y);
      break;
    // DEC
    case 0xC6:
    case 0xD6:
    case 0xCE:
    case 0xDE:
      compare(lookupTable[opcode].mode, this->Y);
      break;
    // DEX
    case 0xCA:
      DECX();
      break;
    // DEY
    case 0x88:
      DECY();
      break;
    // EOR
    case 0x49:
    case 0x45:
    case 0x55:
    case 0x4D:
    case 0x5D:
    case 0x59:
    case 0x41:
    case 0x51:
      EOR(lookupTable[opcode].mode);
      break;
    // INC
    case 0xE6:
    case 0xF6:
    case 0xEE:
    case 0xFE:
      INC(lookupTable[opcode].mode);
      break;
    // INY
    case 0xC8:
      INCY();
      break;
    // JMP
    case 0x4C:
    case 0x6C:
      JMP(lookupTable[opcode].mode);
      break;
    // JSR
    case 0x20:
      JSR(lookupTable[opcode].mode);
      break;
    // LDX
    case 0xA2:
    case 0xA6:
    case 0xB6:
    case 0xAE:
    case 0xBE:
      LDX(lookupTable[opcode].mode);
      break;
    // LSR
    case 0x4A:
      LSRAccumulator();
      break;
    case 0x46:
    case 0x56:
    case 0x4E:
    case 0x5E:
      LSR(lookupTable[opcode].mode);
      break;
    // NOP
    case 0xEA:
      break;
    // ORA
    case 0x09:
    case 0x05:
    case 0x15:
    case 0x0D:
    case 0x1D:
    case 0x19:
    case 0x01:
    case 0x11:
      ORA(lookupTable[opcode].mode);
      break;
    // PHA
    case 0x48:
      PHA();
      break;
    // PHP
    case 0x08:
      PHP();
      break;
    // PLA
    case 0x68:
      PLA();
      break;
    // PLP
    case 0x28:
      PLP();
      break;
    // ROL
    case 0x2A:
      ROLAccumulator();
      break;
    case 0x26:
    case 0x36:
    case 0x2E:
    case 0x3E:
      ROL(lookupTable[opcode].mode);
      break;
    // ROR
    case 0x6A:
      RORAccumulator();
      break;
    case 0x66:
    case 0x76:
    case 0x6E:
    case 0x7E:
      ROR(lookupTable[opcode].mode);
      break;
    // RTI
    case 0x40:
      RTI();
      break;
    case 0x60:
      RTS();
      break;
    // SBC
    case 0xE9:
    case 0xE5:
    case 0xF5:
    case 0xED:
    case 0xFD:
    case 0xF9:
    case 0xE1:
    case 0xF1:
      SBC(lookupTable[opcode].mode);
      break;
    // SEC
    case 0x38:
      SEC();
      break;
    // SED
    case 0xF8:
      SED();
      break;
    // SEI
    case 0x78:
      SEI();
      break;
    // STX
    case 0x86:
    case 0x96:
    case 0x8E:
      STX(lookupTable[opcode].mode);
      break;
    // STY
    case 0x84:
    case 0x94:
    case 0x8C:
      STY(lookupTable[opcode].mode);
      break;
    // TAY
    case 0xA8:
      TAY();
      break;
    // TSX
    case 0xBA:
      TSX();
      break;
    // TXA
    case 0x8A:
      TXA();
      break;
    // TXS
    case 0x9A:
      TXS();
      break;
    // TYA
    case 0x98:
      TYA();
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
    return static_cast<uint16_t>(readFromMemory(this->PC));
  case ZeroPage_X:
    return static_cast<uint16_t>(static_cast<uint8_t>(readFromMemory(this->PC) + this->X));
  case ZeroPage_Y:
    return static_cast<uint16_t>(static_cast<uint8_t>(readFromMemory(this->PC) + this->Y));
  case Absolute:
    return readShortFromMemory(this->PC);
  case Absolute_X:
    return static_cast<uint16_t>(readShortFromMemory(this->PC) + this->X);
  case Absolute_Y:
    return static_cast<uint16_t>(readShortFromMemory(this->PC) + this->Y);
  case Indirect_X: {
    uint8_t base = readFromMemory(this->PC);
    uint8_t pointer = static_cast<uint8_t>(base + this->X);
    uint16_t lo = readFromMemory(pointer);
    uint16_t high = readFromMemory(static_cast<uint8_t>(pointer + 1));
    return ((high << 8) | lo);
  }
  case Indirect_Y: {
    uint8_t pointer = readFromMemory(this->PC);
    uint16_t lo = readFromMemory(pointer);
    uint16_t high = readFromMemory(static_cast<uint8_t>(pointer + 1));
    return static_cast<uint16_t>(((high << 8) | lo) + this->Y);
  }
  case Indirect: {
    uint16_t pointer = readShortFromMemory(this->PC);
    uint16_t lo, hi;
    if ((pointer & 0x00FF) == 0x00FF) {
      lo = readFromMemory(pointer);
      hi = readFromMemory(pointer & 0xFF00);
    } else {
      lo = readFromMemory(pointer);
      hi = readFromMemory(pointer + 1);
    }
    return ((hi << 8) | lo);
  }
  case NoneAddressing:
    // std::cout << "Error: Addressing mode not found" << "\n";
    return 0xFFFF;
  default:
    std::cout << "Error: Addressing mode not found" << "\n";
    return -1;
  }
}

void CPU::reset() {
  this->PC = readShortFromMemory(0xFFFC);
  this->SP = 0xFD;
  this->S = 0;
  this->S |= FLAGS::U;
  this->S |= FLAGS::B;
  this->A = 0;
  this->X = 0;
}
