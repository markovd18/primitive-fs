cmake_minimum_required(VERSION 3.0)
project(primitive_fs)

#setting the language standard
set(CMAKE_CXX_STANDARD 17)

#setting variable for src directory
set(SRC ${PROJECT_SOURCE_DIR}/src)
#setting main src directory
set(MAIN_SRC ${SRC}/main)

#finding all source files in src/main directory
file(GLOB COMMON ${MAIN_SRC}/common/*.h ${MAIN_SRC}/common/*.cpp)
file(GLOB APP ${MAIN_SRC}/app/*.h ${MAIN_SRC}/app/*.cpp)
file(GLOB FS ${MAIN_SRC}/fs/*.h ${FS}/fs/*.cpp)
file(GLOB COMMAND ${MAIN_SRC}/command/*.h ${MAIN_SRC}/command/*.cpp)

#finding all source files in src directory
file(GLOB MAIN ${COMMON} ${APP} ${FS} ${COMMAND} ${MAIN_SRC}/main.cpp)
file(GLOB UTILS ${SRC}/utils/*.h ${SRC}/utils/*.cpp)

#creating one file with all sources for convenience
file(GLOB SOURCES ${MAIN} ${UTILS})
#adding library with file system function definitions
add_library(FUNCTIONS STATIC ${MAIN_SRC}/command/function.cpp)
#creating executable
add_executable(primitive_fs ${SOURCES})

#linking the library with executable
target_link_libraries(primitive_fs FUNCTIONS)
#setting output directory for generated executable to the project root
set_target_properties(primitive_fs PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})