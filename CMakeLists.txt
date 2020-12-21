#
# Author: markovd@students.zcu.cz
#

cmake_minimum_required(VERSION 3.0)
project(primitive_fs)

#setting the language standard
set(CMAKE_CXX_STANDARD 17)

#setting variable for src directory
set(SRC ${PROJECT_SOURCE_DIR}/src)

#finding all source files in src directory
file(GLOB COMMON ${SRC}/common/*.h ${SRC}/common/*.cpp)
file(GLOB APP ${SRC}/app/*.h ${SRC}/app/*.cpp)
file(GLOB FS ${SRC}/fs/*.h ${SRC}/fs/*.cpp)
file(GLOB COMMAND ${SRC}/command/*.h ${SRC}/command/*.cpp)
file(GLOB UTILS ${SRC}/utils/*.h ${SRC}/utils/*.cpp)

#creating one file with all sources for convenience
file(GLOB SOURCES ${APP} ${COMMAND} ${COMMON} ${FS} ${UTILS})
#adding libraries
add_library(FUNCTIONS ${SRC}/command/function.cpp)
add_library(FILE_SYSTEM ${SRC}/fs/FileSystem.cpp)
add_library(FILE_DATA ${SRC}/fs/FileData.cpp)
#creating executable
add_executable(primitive_fs ${SOURCES})

#linking the library with executable
target_link_libraries(primitive_fs PUBLIC FUNCTIONS FILE_SYSTEM FILE_DATA)
#setting output directory for generated executable to the project root
set_target_properties(primitive_fs PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})