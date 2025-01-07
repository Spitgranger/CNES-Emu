#include "cpu.hpp"
#include <string>

std::string traceCpuState(CPU &cpu) {
  uint8_t opcode = cpu.readFromMemory(cpu.PC);
  CPU::instruction instruction = cpu.lookupTable[opcode];
}
