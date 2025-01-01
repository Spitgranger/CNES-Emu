#include "cpu.hpp"
#include <cstdint>
#include <gtest/gtest.h>

class CPUTest : public ::testing::Test {
protected:
  CPU cpu;

  void SetUp() override {
    cpu.S = 0; // Clear the flags before each test
  }
};

TEST_F(CPUTest, TestLDA) {
  uint8_t program[3] = {0xA9, 0x05, 0x00};
  cpu.loadProgramAndRun(program, 3);
  EXPECT_EQ(cpu.A, 0x05);
  // Expect zero flag 0
  EXPECT_EQ(cpu.S & 0b00000010, 0b00);
  // Expect negative flag zero
  EXPECT_EQ(cpu.S & 0b10000000, 0b00);
}

TEST_F(CPUTest, TestLDAFromMemory) {
  cpu.writeToMemory(0x10, 0x55);
  uint8_t program[3] = {0xA5, 0x10, 0x00};
  cpu.loadProgramAndRun(program, 3);
  EXPECT_EQ(cpu.A, 0x55);
}

TEST_F(CPUTest, TestLDAZeroFlag) {
  uint8_t program[3] = {0xA9, 0x00, 0x00};
  cpu.loadProgramAndRun(program, 3);
  EXPECT_EQ(cpu.A, 0x00);
  // Expect zero flag 1
  EXPECT_EQ(cpu.S & 0b00000010, 0b10);
}

TEST_F(CPUTest, TestTAX) {
  cpu.A = 10;
  uint8_t program[4] = {0xA9, 0x10, 0xAA, 0x00};
  cpu.loadProgramAndRun(program, 4);
  // Expect value of 10 in register X
  EXPECT_EQ(cpu.X, 0x10);
  // expect zero flag 0
  EXPECT_EQ(cpu.S & 0b00000010, 0b00);
}

TEST_F(CPUTest, TestSimpleProgramFiveOps) {
  cpu.A = 10;
  uint8_t program[5] = {0xA9, 0xC0, 0xAA, 0xE8, 0x00};
  cpu.loadProgramAndRun(program, 5);
  // Expect value of c1 in register X
  EXPECT_EQ(cpu.X, 0xC1);
}

TEST_F(CPUTest, TestINXOverflow) {
  cpu.X = 0xFF;
  uint8_t program[6] = {0xA9, 0xFF, 0xAA, 0xE8, 0xE8, 0x00};
  cpu.loadProgramAndRun(program, 6);
  // Expect value of c1 in register X
  EXPECT_EQ(cpu.X, 1);
}

TEST_F(CPUTest, TestADCBasicAdditionWithoutCarry) {
  cpu.A = 0x10;            // 16
  cpu.S &= ~CPU::FLAGS::C; // Clear carry flag

  // Simulate an operand to be added.
  uint8_t program[5] = {0xA9, 0x10, 0x69, 0x20, 0x00};
  cpu.loadProgramAndRun(program, 5);

  // Expect accumulator to be 16 + 32 = 48
  EXPECT_EQ(cpu.A, 0x30);              // 0x30 = 48
  EXPECT_EQ(cpu.S & CPU::FLAGS::C, 0); // Ensure carry flag is not set
}

TEST_F(CPUTest, TestADCCarryFlagSet) {
  cpu.A = 0xFF;           // Maximum value for an 8-bit unsigned number
  cpu.S |= CPU::FLAGS::C; // Set carry flag

  uint8_t program[6] = {0xA9, 0xFF, 0x38, 0x69, 0x01, 0x00};
  cpu.loadProgramAndRun(program, 6);

  // Expect accumulator to be 0x00 (256 % 257 = 0) with carry flag set
  EXPECT_EQ(cpu.A, 0x01);
  EXPECT_EQ(cpu.S & CPU::FLAGS::C, CPU::FLAGS::C); // Carry flag should be set
}

TEST_F(CPUTest, TestADCSignedOverflowFlagSet) {
  cpu.A = 0x7F;           // 127 (positive)
  uint8_t operand = 0x01; // 1 (positive)

  // Perform ADC with carry (no carry flag set in this case)
  cpu.S &= ~CPU::FLAGS::C;

  uint8_t program[6] = {0xA9, 0x7F, 0x18, 0x69, 0x01, 0x00};
  cpu.loadProgramAndRun(program, 6);

  // Expect an overflow, because 127 + 1 = 128, which exceeds the signed 8-bit
  // range
  EXPECT_EQ(cpu.A, 0x80); // Result is -128 (overflow)
  EXPECT_EQ(cpu.S & CPU::FLAGS::V,
            CPU::FLAGS::V); // Overflow flag should be set
}

TEST_F(CPUTest, TestADCSignedOverflowNegativeResult) {
  cpu.A = 0x80;            // -128 (signed)
  cpu.S &= ~CPU::FLAGS::C; // Clear carry flag

  uint8_t program[6] = {0xA9, 0x80, 0x18, 0x69, 0xFF, 0x00};
  cpu.loadProgramAndRun(program, 6);

  // Expect an overflow, because -128 + (-1) = -129, which is out of the range
  EXPECT_EQ(cpu.A, 0x7F); // Result is 127 (overflow)
  EXPECT_EQ(cpu.S & CPU::FLAGS::V,
            CPU::FLAGS::V); // Overflow flag should be set
}

TEST_F(CPUTest, TestADCAdditionWithZeroOperand) {
  cpu.A = 0x50; // 80

  uint8_t program[5] = {0xA9, 0x50, 0x69, 0x00, 0x00};
  cpu.loadProgramAndRun(program, 5);

  // Expect the accumulator to remain the same, because we're adding zero
  EXPECT_EQ(cpu.A, 0x50);              // No change
  EXPECT_EQ(cpu.S & CPU::FLAGS::C, 0); // Carry flag should not be set
  EXPECT_EQ(cpu.S & CPU::FLAGS::V, 0); // Overflow flag should not be set
}

TEST_F(CPUTest, TestADCNegativeAddition) {
  cpu.A = 0xF0; // -16 in two's complement

  cpu.S &= ~CPU::FLAGS::C; // Clear carry flag

  uint8_t program[6] = {0xA9, 0xF0, 0x18, 0x69, 0xF0, 0x00};
  cpu.loadProgramAndRun(program, 6);

  // Expected result: -16 + (-16) = -32
  EXPECT_EQ(cpu.A, 0xE0);              // -32 in two's complement
  EXPECT_EQ(cpu.S & CPU::FLAGS::C, 1); // Carry flag should not be set
  EXPECT_EQ(cpu.S & CPU::FLAGS::V, 0); // Overflow flag should not be set
}
