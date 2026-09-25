#include "cpu.hpp"
#include <cstdint>
#include <gtest/gtest.h>

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

  //void SetUp() override {
  //  cpu.P = 0; // Clear the flags before each test
  //}
};

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
