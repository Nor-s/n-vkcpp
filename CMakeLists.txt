cmake_minimum_required(VERSION 3.21)

message("Using CMake version ${CMAKE_VERSION}")

project(
  n-vkcpp
  LANGUAGES CXX
  VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

message(STATUS "Compiler")
message(STATUS " - ID       \t: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " - Version  \t: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " - Path     \t: ${CMAKE_CXX_COMPILER}")

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(./test)