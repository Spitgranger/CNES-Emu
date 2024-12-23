#include "cpu.hpp"

CPU::CPU() {
  CPU::A = 0x00;
  CPU::X = 0x00;
  CPU::Y = 0x00;
  CPU::S = 0x00;
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

void CPU::interpret(const uint8_t program[]) {
  this->PC = 0x0000;
  for (;;) {
    uint8_t opcode = program[this->PC];
    this->PC++;
    switch (opcode) {
    // BRK
    case 0x00:
      BRK();
      return;
    // LDA
    case 0xA9:
      LDA(program[this->PC]);
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

