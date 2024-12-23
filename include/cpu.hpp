#pragma once
#include <cstdint>

class CPU {
public:
  CPU();
  ~CPU();
  // 8 bit registers
  uint8_t A, X, Y, S, P;
  uint16_t SP, PC;
  // 16bit address bus width, 64kib total of byte addressable memory
  uint8_t memory[0xFFFF];

  enum FLAGS {
    C = (1 << 0),
    Z = (1 << 1),
    I = (1 << 2),
    D = (1 << 3),
    B = (1 << 4),
    U = (1 << 5),
    V = (1 << 6),
    N = (1 << 7)
  };

  uint8_t LDA(uint8_t param);
  uint8_t BRK();
  uint8_t TAX();
  uint8_t INX();

  // This method is for testing, receives programs as a seperate input stream
  void interpret(const uint8_t program[]);
};
