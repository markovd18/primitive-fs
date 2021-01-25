#
# Author: markovd@students.zcu.cz
#

cmake_minimum_required(VERSION 3.0)
project(primitive_fs)

#setting the language standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#finding all source files in src directory
file(GLOB_RECURSE COMMON src/common/*.h src/common/*.cpp)
file(GLOB_RECURSE APP src/app/*.h src/app/*.cpp)
file(GLOB_RECURSE FS src/fs/*.h src/fs/*.cpp)
file(GLOB_RECURSE COMMAND src/command/*.h src/command/*.cpp)
file(GLOB_RECURSE UTILS src/utils/*.h src/utils/*.cpp)

#creating one file with all sources for convenience
set(SOURCES ${APP} ${COMMAND} ${COMMON} ${FS} ${UTILS})

#creating executable with sources
add_executable(primitive_fs ${SOURCES})

#setting output directory for generated executable to the project root
set_target_properties(primitive_fs PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

target_link_libraries(primitive_fs stdc++fs)