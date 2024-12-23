cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
project(nes)
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)
add_executable(nes src/main.cpp src/cpu.cpp)
target_include_directories(nes PRIVATE include)

enable_testing()

add_executable(
  cpu_test
  test/cpu_test.cpp
  src/cpu.cpp
)
target_include_directories(cpu_test PRIVATE include)
target_link_libraries(
  cpu_test
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(cpu_test)
