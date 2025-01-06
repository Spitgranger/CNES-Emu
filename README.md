# CNES-Emu

This is yet another implementation of the a NES enumator. The purpose of this
project is not to be a fancy or smart implementation, but rather an exercise to
learn more about computer architecture, C++ testing, and writing idiomatic code.

## Tools and Libraries Used
- GoogleTest
- CMake
- SDL

## Project Structure
```
- src (all project source files)
- include (all project header files)
- test (all project test source files)
```

## Know Issues / TODO
- Tests are failing as the CPU constructor was changed
- Program counter is currently hardcoded to reset to 0x8600 (first instruction
of test rom)
