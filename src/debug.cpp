#include "cpu.hpp"
#include <string>
#include <vector>

std::string traceCpuState(CPU &cpu) {
  // The format of the output string should be PC/CPU OPCODE/OPCODE IN ASS/IF
  // INDIRECT +X OR +Y/REST OF REGISTERS/CPU PPU CLOCK CYCLES
  uint8_t opcode = cpu.readFromMemory(cpu.PC);
  uint16_t programCounter = cpu.PC;
  CPU::instruction instruction = cpu.lookupTable[opcode];
  std::string assemblyName = instruction.name;
  uint8_t sizeOfInstruction = instruction.bytes;
  std::vector<uint8_t> hexDump;
  hexDump.push_back(opcode);

  uint16_t operandAddress;
  uint8_t operandValue;

  if (instruction.mode == CPU::ADDRESSING::Immediate || instruction.mode == CPU::ADDRESSING::NoneAddressing) {
    operandAddress = 0;
    operandValue = 0;
  } else {
    operandAddress = cpu.getAbsoluteAddress(instruction.mode ,programCounter + 1);
    operandValue = cpu.readFromMemory(operandAddress);
  }
  
}
