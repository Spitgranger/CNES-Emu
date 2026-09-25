#include "cpu.hpp"
#include <algorithm>
#include <cstdint>
#include <gtest/gtest.h>
#include <stdexcept>

class CPUTest : public ::testing::Test {
protected:

  CPU createSystem(const std::vector<uint8_t>& program) {
    constexpr size_t headerSize = 16;
    constexpr size_t prgSize = 0x4000; // 16 KiB
    constexpr size_t chrSize = 0x2000; // 8 KiB

    // Reserve the final six PRG bytes for interrupt vectors.
    if (program.size() > prgSize - 6) {
      throw std::invalid_argument("Test program is too large");
    }

    std::vector<uint8_t> cartridge(headerSize + prgSize + chrSize, 0);

    // iNES header: mapper 0, one PRG bank, one CHR bank.
    cartridge[0] = 0x4E; // N
    cartridge[1] = 0x45; // E
    cartridge[2] = 0x53; // S
    cartridge[3] = 0x1A;
    cartridge[4] = 1;
    cartridge[5] = 1;

    // PRG begins at CPU address $8000.
    std::copy(program.begin(), program.end(),
              cartridge.begin() + headerSize);

    // The 16 KiB PRG bank is mirrored at $C000.
    // CPU address $FFFC therefore maps to PRG offset $3FFC.
    cartridge[headerSize + 0x3FFC] = 0x00;
    cartridge[headerSize + 0x3FFD] = 0x80;

    return CPU(Bus(cartridge));
  }

  void verifyLDY(std::vector<uint8_t> program, uint16_t operandAddress,
                 uint8_t index = 0) {
    const bool immediate = program[0] == 0xA0;
    const auto expectedPC = static_cast<uint16_t>(0x8000 + program.size());
    constexpr uint8_t unchangedFlags = CPU::FLAGS::C | CPU::FLAGS::I |
                                       CPU::FLAGS::D | CPU::FLAGS::V;

    for (uint8_t value : {0x00, 0x42, 0x80, 0xFF}) {
      for (bool flagsInitiallySet : {false, true}) {
        SCOPED_TRACE(::testing::Message()
                     << "value=" << static_cast<int>(value)
                     << ", flagsInitiallySet=" << flagsInitiallySet);
        if (immediate) {
          program[1] = value;
        }
        CPU cpu = createSystem(program);
        cpu.reset();
        cpu.A = 0x35;
        cpu.X = index;
        cpu.Y = static_cast<uint8_t>(~value);
        // BRK currently stops the interpreter and sets B; start with B set
        // so the status assertion below isolates LDY's changes.
        cpu.P = CPU::FLAGS::B | CPU::FLAGS::U;
        if (flagsInitiallySet) {
          cpu.P |= unchangedFlags | CPU::FLAGS::Z | CPU::FLAGS::N;
        }
        const uint8_t initialStatus = cpu.P;
        const uint8_t initialSP = cpu.SP;
        if (!immediate) {
          cpu.writeToMemory(operandAddress, value);
        }

        cpu.interpret();

        uint8_t expectedStatus = initialStatus & ~(CPU::FLAGS::Z | CPU::FLAGS::N);
        if (value == 0) {
          expectedStatus |= CPU::FLAGS::Z;
        }
        if (value & 0x80) {
          expectedStatus |= CPU::FLAGS::N;
        }
        EXPECT_EQ(cpu.Y, value);
        EXPECT_EQ(cpu.P, expectedStatus);
        EXPECT_EQ(cpu.A, 0x35);
        EXPECT_EQ(cpu.X, index);
        EXPECT_EQ(cpu.SP, initialSP);
        EXPECT_EQ(cpu.PC, expectedPC);
        if (!immediate) {
          EXPECT_EQ(cpu.readFromMemory(operandAddress), value);
        }
      }
    }
  }

  void verifyRotate(bool rotateLeft, bool accumulator) {
    struct Case {
      uint8_t input;
      bool carryIn;
      uint8_t result;
      uint8_t flags; // Expected C, Z, and N.
    };
    const std::vector<Case> cases = rotateLeft
        ? std::vector<Case>{{0x00, true, 0x01, 0},
                            {0x80, false, 0x00, CPU::FLAGS::C | CPU::FLAGS::Z},
                            {0x40, false, 0x80, CPU::FLAGS::N},
                            {0x80, true, 0x01, CPU::FLAGS::C},
                            {0x00, false, 0x00, CPU::FLAGS::Z},
                            {0xFF, true, 0xFF, CPU::FLAGS::C | CPU::FLAGS::N}}
        : std::vector<Case>{{0x00, true, 0x80, CPU::FLAGS::N},
                            {0x01, false, 0x00, CPU::FLAGS::C | CPU::FLAGS::Z},
                            {0x80, false, 0x40, 0},
                            {0x01, true, 0x80, CPU::FLAGS::C | CPU::FLAGS::N},
                            {0x00, false, 0x00, CPU::FLAGS::Z},
                            {0xFF, true, 0xFF, CPU::FLAGS::C | CPU::FLAGS::N}};
    struct Mode {
      std::vector<uint8_t> program;
      uint16_t address;
      uint8_t index;
    };
    // ROR opcodes are the corresponding ROL opcodes plus $40.
    std::vector<Mode> modes = accumulator
        ? std::vector<Mode>{{{0x2A, 0x00}, 0x0042, 0x03}}
        : std::vector<Mode>{{{0x26, 0x42, 0x00}, 0x0042, 0x03},
                            {{0x36, 0xFF, 0x00}, 0x0002, 0x03},
                            {{0x2E, 0x23, 0x01, 0x00}, 0x0123, 0x03},
                            {{0x3E, 0xFF, 0x01, 0x00}, 0x0202, 0x03}};
    for (auto mode : modes) {
      if (!rotateLeft) {
        mode.program[0] += 0x40;
      }
      for (const auto& test : cases) {
        for (bool otherFlagsSet : {false, true}) {
          SCOPED_TRACE(::testing::Message()
                       << "opcode=" << static_cast<int>(mode.program[0])
                       << ", input=" << static_cast<int>(test.input)
                       << ", carryIn=" << test.carryIn
                       << ", otherFlagsSet=" << otherFlagsSet);
          CPU cpu = createSystem(mode.program);
          cpu.reset();
          cpu.A = accumulator ? test.input : 0x35;
          cpu.X = mode.index;
          cpu.Y = 0x57;
          // B is already set because the terminating BRK sets it.
          uint8_t preservedFlags = CPU::FLAGS::B | CPU::FLAGS::U;
          if (otherFlagsSet) {
            preservedFlags |= CPU::FLAGS::I | CPU::FLAGS::D | CPU::FLAGS::V;
          }
          // Start Z/N opposite to their expected values to test both clearing
          // and setting, independent of the incoming carry.
          cpu.P = preservedFlags |
                  ((~test.flags) & (CPU::FLAGS::Z | CPU::FLAGS::N)) |
                  (test.carryIn ? CPU::FLAGS::C : 0);
          const uint8_t initialSP = cpu.SP;
          cpu.writeToMemory(mode.address, accumulator ? 0x5A : test.input);

          cpu.interpret();

          EXPECT_EQ(cpu.A, accumulator ? test.result : 0x35);
          EXPECT_EQ(cpu.readFromMemory(mode.address),
                    accumulator ? 0x5A : test.result);
          EXPECT_EQ(cpu.P, preservedFlags | test.flags);
          EXPECT_EQ(cpu.X, mode.index);
          EXPECT_EQ(cpu.Y, 0x57);
          EXPECT_EQ(cpu.SP, initialSP);
          EXPECT_EQ(cpu.PC, 0x8000 + mode.program.size());
        }
      }
    }
  }

  //void SetUp() override {
  //  cpu.P = 0; // Clear the flags before each test
  //}
};

TEST_F(CPUTest, TestROLAccumulatorCarryAndFlags) {
  verifyRotate(true, true);
}

TEST_F(CPUTest, TestROLMemoryCarryAndFlags) {
  verifyRotate(true, false);
}

TEST_F(CPUTest, TestRORAccumulatorCarryAndFlags) {
  verifyRotate(false, true);
}

TEST_F(CPUTest, TestRORMemoryCarryAndFlags) {
  verifyRotate(false, false);
}

TEST_F(CPUTest, TestLDYImmediate) {
  verifyLDY({0xA0, 0x00, 0x00}, 0);
}

TEST_F(CPUTest, TestLDYZeroPage) {
  verifyLDY({0xA4, 0x42, 0x00}, 0x0042);
}

TEST_F(CPUTest, TestLDYZeroPageX) {
  verifyLDY({0xB4, 0x40, 0x00}, 0x0045, 0x05);
}

TEST_F(CPUTest, TestLDYZeroPageXWraps) {
  verifyLDY({0xB4, 0xFF, 0x00}, 0x0001, 0x02);
}

TEST_F(CPUTest, TestLDYAbsolute) {
  verifyLDY({0xAC, 0x23, 0x01, 0x00}, 0x0123);
}

TEST_F(CPUTest, TestLDYAbsoluteX) {
  verifyLDY({0xBC, 0x20, 0x01, 0x00}, 0x0125, 0x05);
}

TEST_F(CPUTest, TestLDYAbsoluteXCrossesPage) {
  verifyLDY({0xBC, 0xFF, 0x01, 0x00}, 0x0201, 0x02);
}

TEST_F(CPUTest, TestLDA) {
  std::vector<uint8_t> program = {0xA9, 0x05, 0x00};
  //cpu.loadProgramAndRun(program, 3);
  CPU cpu = createSystem(program);
  cpu.reset();
  cpu.interpret();
  EXPECT_EQ(cpu.A, 0x05);
  // Expect zero flag 0
  EXPECT_EQ(cpu.P & 0b00000010, 0b00);
  // Expect negative flag zero
  EXPECT_EQ(cpu.P & 0b10000000, 0b00);
}

TEST_F(CPUTest, TestLDAFromMemory) {
  std::vector<uint8_t> program = {0xA5, 0x10, 0x00};
  CPU cpu = createSystem(program);
  cpu.reset();
  cpu.writeToMemory(0x10, 0x55);
  cpu.interpret();
  //cpu.loadProgramAndRun(program, 3);
  EXPECT_EQ(cpu.A, 0x55);
}

TEST_F(CPUTest, TestLDAZeroFlag) {
  std::vector<uint8_t> program = {0xA9, 0x00, 0x00};
  CPU cpu = createSystem(program);
  cpu.reset();
  cpu.interpret();
  //cpu.loadProgramAndRun(program, 3);
  EXPECT_EQ(cpu.A, 0x00);
  // Expect zero flag 1
  EXPECT_EQ(cpu.P & 0b00000010, 0b10);
}

TEST_F(CPUTest, TestTAX) {
  std::vector<uint8_t> program = {0xA9, 0x10, 0xAA, 0x00};
  CPU cpu = createSystem(program);
  cpu.reset();
  cpu.A = 10;
  cpu.interpret();
  //cpu.loadProgramAndRun(program, 4);
  // Expect value of 10 in register X
  EXPECT_EQ(cpu.X, 0x10);
  // expect zero flag 0
  EXPECT_EQ(cpu.P & 0b00000010, 0b00);
}

TEST_F(CPUTest, TestSimpleProgramFiveOps) {
  std::vector<uint8_t> program = {0xA9, 0xC0, 0xAA, 0xE8, 0x00};
  CPU cpu = createSystem(program);
  cpu.reset();
  cpu.A = 10;
  cpu.interpret();
  //cpu.loadProgramAndRun(program, 5);
  // Expect value of c1 in register X
  EXPECT_EQ(cpu.X, 0xC1);
}

TEST_F(CPUTest, TestINXOverflow) {
  std::vector<uint8_t> program = {0xA9, 0xFF, 0xAA, 0xE8, 0xE8, 0x00};
  CPU cpu = createSystem(program);
  cpu.reset();
  cpu.X = 0xFF;
  cpu.interpret();
  //cpu.loadProgramAndRun(program, 6);
  // Expect value of c1 in register X
  EXPECT_EQ(cpu.X, 1);
}

TEST_F(CPUTest, TestADCBasicAdditionWithoutCarry) {
  // Simulate an operand to be added.
  std::vector<uint8_t> program = {0xA9, 0x10, 0x69, 0x20, 0x00};
  CPU cpu = createSystem(program);
  cpu.reset();
  cpu.A = 0x10;            // 16
  cpu.P &= ~CPU::FLAGS::C; // Clear carry flag
  cpu.interpret();

  //cpu.loadProgramAndRun(program, 5);

  // Expect accumulator to be 16 + 32 = 48
  EXPECT_EQ(cpu.A, 0x30);              // 0x30 = 48
  EXPECT_EQ(cpu.P & CPU::FLAGS::C, 0); // Ensure carry flag is not set
}

TEST_F(CPUTest, TestADCCarryFlagSet) {
  std::vector<uint8_t> program = {0xA9, 0xFF, 0x38, 0x69, 0x01, 0x00};
  CPU cpu = createSystem(program);
  cpu.reset();
  cpu.A = 0xFF;           // Maximum value for an 8-bit unsigned number
  cpu.P |= CPU::FLAGS::C; // Set carry flag
  cpu.interpret();

  //cpu.loadProgramAndRun(program, 6);

  // Expect accumulator to be 0x00 (256 % 257 = 0) with carry flag set
  EXPECT_EQ(cpu.A, 0x01);
  EXPECT_EQ(cpu.P & CPU::FLAGS::C, CPU::FLAGS::C); // Carry flag should be set
}

TEST_F(CPUTest, TestADCSignedOverflowFlagSet) {
  std::vector<uint8_t> program = {0xA9, 0x7F, 0x18, 0x69, 0x01, 0x00};
  CPU cpu = createSystem(program);
  cpu.reset();
  cpu.A = 0x7F;           // 127 (positive)
  uint8_t operand = 0x01; // 1 (positive)

  // Perform ADC with carry (no carry flag set in this case)
  cpu.P &= ~CPU::FLAGS::C;
  cpu.interpret();
  //cpu.loadProgramAndRun(program, 6);

  // Expect an overflow, because 127 + 1 = 128, which exceeds the signed 8-bit
  // range
  EXPECT_EQ(cpu.A, 0x80); // Result is -128 (overflow)
  EXPECT_EQ(cpu.P & CPU::FLAGS::V,
            CPU::FLAGS::V); // Overflow flag should be set
}

TEST_F(CPUTest, TestADCSignedOverflowNegativeResult) {
  std::vector<uint8_t> program = {0xA9, 0x80, 0x18, 0x69, 0xFF, 0x00};
  CPU cpu = createSystem(program);
  cpu.reset();
  cpu.A = 0x80;            // -128 (signed)
  cpu.P &= ~CPU::FLAGS::C; // Clear carry flag
  cpu.interpret();

  //cpu.loadProgramAndRun(program, 6);

  // Expect an overflow, because -128 + (-1) = -129, which is out of the range
  EXPECT_EQ(cpu.A, 0x7F); // Result is 127 (overflow)
  EXPECT_EQ(cpu.P & CPU::FLAGS::V,
            CPU::FLAGS::V); // Overflow flag should be set
}

TEST_F(CPUTest, TestADCAdditionWithZeroOperand) {
  std::vector<uint8_t> program = {0xA9, 0x50, 0x69, 0x00, 0x00};
  CPU cpu = createSystem(program);
  cpu.reset();
  cpu.A = 0x50; // 80
  cpu.interpret();

  //cpu.loadProgramAndRun(program, 5);

  // Expect the accumulator to remain the same, because we're adding zero
  EXPECT_EQ(cpu.A, 0x50);              // No change
  EXPECT_EQ(cpu.P & CPU::FLAGS::C, 0); // Carry flag should not be set
  EXPECT_EQ(cpu.P & CPU::FLAGS::V, 0); // Overflow flag should not be set
}

TEST_F(CPUTest, TestADCNegativeAddition) {
  std::vector<uint8_t> program = {0xA9, 0xF0, 0x18, 0x69, 0xF0, 0x00};
  CPU cpu = createSystem(program);
  cpu.reset();
  cpu.A = 0xF0; // -16 in two's complement

  cpu.P &= ~CPU::FLAGS::C; // Clear carry flag
  cpu.interpret();

  //cpu.loadProgramAndRun(program, 6);

  // Expected result: -16 + (-16) = -32
  EXPECT_EQ(cpu.A, 0xE0);              // -32 in two's complement
  EXPECT_EQ(cpu.P & CPU::FLAGS::C, 1); // Carry flag should not be set
  EXPECT_EQ(cpu.P & CPU::FLAGS::V, 0); // Overflow flag should not be set
}
