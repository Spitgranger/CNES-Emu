#include <cstdint>
#include <gtest/gtest.h>
#include "cpu.hpp"

TEST(CPUTest, TestLDA) {
	CPU cpu = CPU();
	uint8_t program[3] = {0xA9, 0x05, 0x00};
	cpu.loadProgramAndRun(program, 3);
	EXPECT_EQ(cpu.A, 0x05);
	// Expect zero flag 0
	EXPECT_EQ(cpu.S & 0b00000010, 0b00);
	// Expect negative flag zero
	EXPECT_EQ(cpu.S & 0b10000000, 0b00);
}

TEST(CPUTest, TestLDAFromMemory) {
	CPU cpu = CPU();
	cpu.writeToMemory(0x10, 0x55);
	uint8_t program[3] = {0xA5, 0x10, 0x00};
	cpu.loadProgramAndRun(program, 3);
	EXPECT_EQ(cpu.A, 0x55);
}

TEST(CPUTest, TestLDAZeroFlag) {
	CPU cpu = CPU();
	uint8_t program[3] = {0xA9, 0x00, 0x00};
	cpu.loadProgramAndRun(program, 3);
	EXPECT_EQ(cpu.A, 0x00);
	// Expect zero flag 1
	EXPECT_EQ(cpu.S & 0b00000010, 0b10);
}

TEST(CPUTest, TestTAX) {
	CPU cpu = CPU();
	cpu.A = 10;
	uint8_t program[2] = {0xAA, 0x00};
	cpu.loadProgramAndRun(program, 2);
	// Expect value of 10 in register X
	EXPECT_EQ(cpu.X, 10);
	// expect zero flag 0
	EXPECT_EQ(cpu.S & 0b00000010, 0b00);
}

TEST(CPUTest, TestSimpleProgramFiveOps) {
	CPU cpu = CPU();
	cpu.A = 10;
	uint8_t program[5] = {0xA9, 0xC0, 0xAA, 0xE8, 0x00};
	cpu.loadProgramAndRun(program, 5);
	// Expect value of c1 in register X
	EXPECT_EQ(cpu.X, 0xC1);
}

TEST(CPUTest, TestINXOverflow) {
	CPU cpu = CPU();
	cpu.X = 0xFF;
	uint8_t program[3] = {0xE8, 0xE8, 0x00};
	cpu.loadProgramAndRun(program, 3);
	// Expect value of c1 in register X
	EXPECT_EQ(cpu.X, 1);
}
