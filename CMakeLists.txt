cmake_minimum_required(VERSION 3.10)
project(FlySentinel)

set(CMAKE_C_STANDARD 11)

add_executable(FlySentinel src/main.cpp src/flySentinel/FlySentinel.cpp src/flySentinel/FlySentinel.h src/coordinator/interface/AbstractFlySentinel.h src/instanceLink/InstanceLink.cpp src/instanceLink/InstanceLink.h src/coordinator/interface/AbstractInstanceLink.h)
