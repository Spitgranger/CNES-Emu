#include "cpu.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

template <typename T, typename U> bool all_equal(const T &t, const U &u) {
  return t == u;
}

template <typename T, typename U, typename... Others>
bool all_equal(const T &t, const U &u, Others const &...args) {
  return (t == u) || all_equal(t, args...);
}

std::string traceCpuState(CPU *cpu) {
  // The format of the output string should be PC/CPU OPCODE/OPCODE IN ASS/IF
  // INDIRECT +X OR +Y/REST OF REGISTERS/CPU PPU CLOCK CYCLES
  uint8_t opcode = cpu->readFromMemory(cpu->PC);
  uint16_t programCounter = cpu->PC;
  CPU::instruction instruction = cpu->lookupTable[opcode];
  std::string assemblyName = instruction.name;
  uint8_t sizeOfInstruction = instruction.bytes;
  std::vector<uint8_t> hexDump;
  hexDump.push_back(opcode);

  uint16_t operandAddress;
  uint8_t operandValue;

  if (instruction.mode == CPU::ADDRESSING::Immediate ||
      instruction.mode == CPU::ADDRESSING::NoneAddressing) {
    operandAddress = 0;
    operandValue = 0;
  } else {
    operandAddress =
        cpu->getAbsoluteAddress(instruction.mode, programCounter + 1);
    operandValue = cpu->readFromMemory(operandAddress);
  }

  std::string tempString;

  switch (instruction.bytes) {
  case 1: {
    if (all_equal(opcode, 0x0a, 0x4a, 0x2a, 0x6a)) {
      tempString = "A ";
    } else {
      tempString = "";
    }
    break;
  }
  case 2: {
    uint8_t address = cpu->readFromMemory(programCounter + 1);
    hexDump.push_back(address);
    switch (instruction.mode) {
    case CPU::ADDRESSING::Immediate: {
      char temp[10];
      std::sprintf(temp, "#$%02x", address);
      tempString = temp;
      break;
    }
    case CPU::ADDRESSING::ZeroPage: {
      char temp[10];
      std::sprintf(temp, "$%02x = %02x", operandAddress, operandValue);
      tempString = temp;
      break;
    }
    case CPU::ADDRESSING::ZeroPage_X: {
      char temp[18];
      std::sprintf(temp, "$%02x,X @ %02x = %02x", address, operandAddress,
                   operandValue);
      tempString = temp;
      break;
    }
    case CPU::ADDRESSING::ZeroPage_Y: {
      char temp[18];
      std::sprintf(temp, "$%02x,Y @ %02x = %02x", address, operandAddress,
                   operandValue);
      tempString = temp;
      break;
    }
    case CPU::ADDRESSING::Indirect_X: {
      char temp[30];
      std::sprintf(temp, "($%02x,X) = %02x @ %04x = %02x", address,
                   static_cast<uint8_t>(address + cpu->X), operandAddress,
                   operandValue);
      tempString = temp;
      break;
    }
    case CPU::ADDRESSING::Indirect_Y: {
      char temp[30];
      std::sprintf(temp, "($%02x),Y = %04x @ %04x = %02x", address,
                   static_cast<uint16_t>(operandAddress - cpu->Y),
                   operandAddress, operandValue);
      tempString = temp;
      break;
    }
    case CPU::ADDRESSING::NoneAddressing: {
      // this will assume local jumps such as BNE, BEQ
      uint16_t branchAddress =
          (programCounter + 2) + static_cast<int8_t>(address);
      char temp[10];
      std::sprintf(temp, "$%04x", branchAddress);
      tempString = temp;
      break;
    }
    default:
      std::cout << "Unknown addressing mode" << std::hex << instruction.opcode
                << "has operation length of two bytes";
      break;
    }
    break;
  }
  case 3: {
    uint16_t lo = cpu->readFromMemory(programCounter + 1);
    uint16_t hi = cpu->readFromMemory(programCounter + 2);
    hexDump.push_back(lo);
    hexDump.push_back(hi);
    uint16_t address = cpu->readShortFromMemory(programCounter + 1);
    switch (instruction.mode) {
    case CPU::ADDRESSING::Indirect:
    case CPU::ADDRESSING::NoneAddressing: {
      uint16_t jumpAddress;
      if (instruction.opcode == 0x6C) {
        if ((address & 0x00FF) == 0x00FF) {
          uint16_t lo = cpu->readFromMemory(address);
          uint16_t hi = cpu->readFromMemory(address * 0xFF00);
          jumpAddress = (hi << 8) | lo;
        } else {
          jumpAddress = cpu->readShortFromMemory(address);
        }
        char temp[20];
        std::sprintf(temp, "($%04x) = %04x", address, jumpAddress);
        tempString = temp;
      }
      char temp[10];
      std::sprintf(temp, "$%04x", address);
      tempString = temp;
      break;
    }
    case CPU::ADDRESSING::Absolute: {
      if (instruction.opcode == 0x4C) {
        char temp[10];
        std::sprintf(temp, "$%04x", address);
        tempString = temp;
        break;
      }
      char temp[15];
      std::sprintf(temp, "$%04x = %02x", operandAddress, operandValue);
      tempString = temp;
      break;
    }
    case CPU::ADDRESSING::Absolute_X: {
      char temp[24];
      std::sprintf(temp, "$%04x,X @ %04x = %02x", address, operandAddress,
                   operandValue);
      tempString = temp;
      break;
    }
    case CPU::ADDRESSING::Absolute_Y: {
      char temp[24];
      std::sprintf(temp, "$%04x,Y @ %04x = %02x", address, operandAddress,
                   operandValue);
      tempString = temp;
      break;
    }
    default:
      std::cout << "Unknown addressing mode" << std::hex << instruction.opcode
                << "has operation length of three bytes";
      break;
    }
    break;
  }
  default:
    tempString = "";
  }
  std::stringstream ss;
  for (size_t i = 0; i < hexDump.size(); i++) {
    ss << std::setw(2) << std::setfill('0') << std::hex
       << static_cast<int>(hexDump[i]);
    if (i != hexDump.size() - 1) {
      ss << " ";
    }
  }
  std::string hexString = ss.str();
  ss.str("");
  ss.clear();
  ss << std::setw(4) << std::setfill(' ') << std::hex << programCounter << "  " << std::setw(8)
     << hexString << " " << std::setw(4) << " " << instruction.name << " "
     << tempString;
  std::string asmString = ss.str();
  ss.str("");
  ss.clear();
  ss << asmString << " " << "A:" << std::setw(2)
     << std::setfill('0') << std::hex << static_cast<int>(cpu->A) << " "
     << "X:" << std::setw(2) << std::setfill('0') << std::hex
     << static_cast<int>(cpu->X) << " " << "Y:" << std::setw(2)
     << std::setfill('0') << std::hex << static_cast<int>(cpu->Y) << " "
     << "P:" << std::setw(2) << std::setfill('0') << std::hex
     << static_cast<int>(cpu->S) << " " << "SP:" << std::setw(2)
     << std::setfill('0') << std::hex << static_cast<int>(cpu->SP);

  // Get the formatted string
  std::string result = ss.str();
  std::transform(result.begin(), result.end(), result.begin(), ::toupper);
  return result;
}
