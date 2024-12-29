#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

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

  enum ADDRESSING {
    Immediate,
    ZeroPage,
    ZeroPage_X,
    ZeroPage_Y,
    Absolute,
    Absolute_X,
    Absolute_Y,
    Indirect_X,
    Indirect_Y,
    Indirect,
    NoneAddressing
  };

  // CPU Instructions
  uint8_t BRK();
  uint8_t TAX();
  uint8_t INX();
  uint8_t STA(ADDRESSING mode);
  uint8_t ADC(ADDRESSING mode);
  uint8_t AND(ADDRESSING mode);
  uint8_t ASL(ADDRESSING mode);
  uint8_t ASLAccumulator();
  void branch(bool condition);
  void compare(ADDRESSING mode, uint8_t reg);
  uint8_t BIT(ADDRESSING mode);
  uint8_t DEC(ADDRESSING mode);
  void DECX();
  void DECY();
  uint8_t EOR(ADDRESSING mode);
  uint8_t INC(ADDRESSING mode);
  void INCX();
  void INCY();
  uint8_t JMP(ADDRESSING mode);
  uint8_t JSR(ADDRESSING mode);
  uint8_t LDA(ADDRESSING mode);
  uint8_t LDX(ADDRESSING mode);
  uint8_t LDY(ADDRESSING mode);
  uint8_t LSR(ADDRESSING mode);
  uint8_t LSRAccumulator();

  // Memory Access
  uint8_t readFromMemory(uint16_t address);
  void writeToMemory(uint16_t address, uint8_t data);
  uint16_t readShortFromMemory(uint16_t address);
  void writeShortToMemory(uint16_t address, uint16_t data);
  void loadProgram(uint8_t program[], uint32_t size);
  void loadProgramAndRun(uint8_t program[], uint32_t size);
  uint16_t getOperandAddress(ADDRESSING mode);

  // This method is for testing, receives programs as a seperate input stream
  void interpret();

  // CPU Functional Methods
  void reset();
  void pushOnStack(uint8_t value);
  uint8_t popFromStack();

private:
  struct instruction {
    // Opcode of the instruction
    uint8_t opcode;
    // mnemonic of english name of instruction
    std::string name;
    // Number of bytes for the instruction including arguments
    uint8_t bytes;
    // NUmber of CPU cycles needed
    uint8_t cycles;
    // Addressing mode
    ADDRESSING mode;
  };
  static std::vector<instruction> opcodeTable;
  std::unordered_map<uint16_t, instruction> lookupTable;
  void setZeroAndNegativeFlags(uint8_t value);
};
