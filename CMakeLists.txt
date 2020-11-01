cmake_minimum_required(VERSION 3.0)
project(primitive_fs)

set(CMAKE_CXX_STANDARD 17)

add_executable(primitive_fs main.cpp)
set_target_properties(primitive_fs PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})