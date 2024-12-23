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
  // CPU Instructions
  uint8_t LDA(uint8_t param);
  uint8_t BRK();
  uint8_t TAX();
  uint8_t INX();

  // Memory Access
  uint8_t readFromMemory(uint16_t address);
  void writeToMemory(uint16_t address, uint8_t data);
  uint16_t readShortFromMemory(uint16_t address);
  void writeShortToMemory(uint16_t address, uint16_t data);
  void loadProgram(uint8_t program[], uint32_t size);
  void loadProgramAndRun(uint8_t program[], uint32_t size);

  // This method is for testing, receives programs as a seperate input stream
  void interpret();

  // CPU Functional Methods
  void reset();
};
