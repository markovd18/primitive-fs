# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/markovda/sw/clion-2020.2.3/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/markovda/sw/clion-2020.2.3/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/markovda/Skola/repos/primitive-fs

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/markovda/Skola/repos/primitive-fs

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/home/markovda/sw/clion-2020.2.3/bin/cmake/linux/bin/cmake --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/home/markovda/sw/clion-2020.2.3/bin/cmake/linux/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/markovda/Skola/repos/primitive-fs/CMakeFiles /home/markovda/Skola/repos/primitive-fs/CMakeFiles/progress.marks
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/markovda/Skola/repos/primitive-fs/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named FUNCTIONS

# Build rule for target.
FUNCTIONS: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 FUNCTIONS
.PHONY : FUNCTIONS

# fast build rule for target.
FUNCTIONS/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/FUNCTIONS.dir/build.make CMakeFiles/FUNCTIONS.dir/build
.PHONY : FUNCTIONS/fast

#=============================================================================
# Target rules for targets named FILE_SYSTEM

# Build rule for target.
FILE_SYSTEM: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 FILE_SYSTEM
.PHONY : FILE_SYSTEM

# fast build rule for target.
FILE_SYSTEM/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/FILE_SYSTEM.dir/build.make CMakeFiles/FILE_SYSTEM.dir/build
.PHONY : FILE_SYSTEM/fast

#=============================================================================
# Target rules for targets named FILE_DATA

# Build rule for target.
FILE_DATA: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 FILE_DATA
.PHONY : FILE_DATA

# fast build rule for target.
FILE_DATA/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/FILE_DATA.dir/build.make CMakeFiles/FILE_DATA.dir/build
.PHONY : FILE_DATA/fast

#=============================================================================
# Target rules for targets named primitive_fs

# Build rule for target.
primitive_fs: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 primitive_fs
.PHONY : primitive_fs

# fast build rule for target.
primitive_fs/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/build
.PHONY : primitive_fs/fast

src/app/PrimitiveFsApp.o: src/app/PrimitiveFsApp.cpp.o

.PHONY : src/app/PrimitiveFsApp.o

# target to build an object file
src/app/PrimitiveFsApp.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/src/app/PrimitiveFsApp.cpp.o
.PHONY : src/app/PrimitiveFsApp.cpp.o

src/app/PrimitiveFsApp.i: src/app/PrimitiveFsApp.cpp.i

.PHONY : src/app/PrimitiveFsApp.i

# target to preprocess a source file
src/app/PrimitiveFsApp.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/src/app/PrimitiveFsApp.cpp.i
.PHONY : src/app/PrimitiveFsApp.cpp.i

src/app/PrimitiveFsApp.s: src/app/PrimitiveFsApp.cpp.s

.PHONY : src/app/PrimitiveFsApp.s

# target to generate assembly for a file
src/app/PrimitiveFsApp.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/src/app/PrimitiveFsApp.cpp.s
.PHONY : src/app/PrimitiveFsApp.cpp.s

src/app/main.o: src/app/main.cpp.o

.PHONY : src/app/main.o

# target to build an object file
src/app/main.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/src/app/main.cpp.o
.PHONY : src/app/main.cpp.o

src/app/main.i: src/app/main.cpp.i

.PHONY : src/app/main.i

# target to preprocess a source file
src/app/main.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/src/app/main.cpp.i
.PHONY : src/app/main.cpp.i

src/app/main.s: src/app/main.cpp.s

.PHONY : src/app/main.s

# target to generate assembly for a file
src/app/main.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/src/app/main.cpp.s
.PHONY : src/app/main.cpp.s

src/command/function.o: src/command/function.cpp.o

.PHONY : src/command/function.o

# target to build an object file
src/command/function.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/FUNCTIONS.dir/build.make CMakeFiles/FUNCTIONS.dir/src/command/function.cpp.o
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/src/command/function.cpp.o
.PHONY : src/command/function.cpp.o

src/command/function.i: src/command/function.cpp.i

.PHONY : src/command/function.i

# target to preprocess a source file
src/command/function.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/FUNCTIONS.dir/build.make CMakeFiles/FUNCTIONS.dir/src/command/function.cpp.i
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/src/command/function.cpp.i
.PHONY : src/command/function.cpp.i

src/command/function.s: src/command/function.cpp.s

.PHONY : src/command/function.s

# target to generate assembly for a file
src/command/function.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/FUNCTIONS.dir/build.make CMakeFiles/FUNCTIONS.dir/src/command/function.cpp.s
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/src/command/function.cpp.s
.PHONY : src/command/function.cpp.s

src/common/structures.o: src/common/structures.cpp.o

.PHONY : src/common/structures.o

# target to build an object file
src/common/structures.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/src/common/structures.cpp.o
.PHONY : src/common/structures.cpp.o

src/common/structures.i: src/common/structures.cpp.i

.PHONY : src/common/structures.i

# target to preprocess a source file
src/common/structures.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/src/common/structures.cpp.i
.PHONY : src/common/structures.cpp.i

src/common/structures.s: src/common/structures.cpp.s

.PHONY : src/common/structures.s

# target to generate assembly for a file
src/common/structures.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/src/common/structures.cpp.s
.PHONY : src/common/structures.cpp.s

src/fs/FileData.o: src/fs/FileData.cpp.o

.PHONY : src/fs/FileData.o

# target to build an object file
src/fs/FileData.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/FILE_DATA.dir/build.make CMakeFiles/FILE_DATA.dir/src/fs/FileData.cpp.o
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/src/fs/FileData.cpp.o
.PHONY : src/fs/FileData.cpp.o

src/fs/FileData.i: src/fs/FileData.cpp.i

.PHONY : src/fs/FileData.i

# target to preprocess a source file
src/fs/FileData.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/FILE_DATA.dir/build.make CMakeFiles/FILE_DATA.dir/src/fs/FileData.cpp.i
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/src/fs/FileData.cpp.i
.PHONY : src/fs/FileData.cpp.i

src/fs/FileData.s: src/fs/FileData.cpp.s

.PHONY : src/fs/FileData.s

# target to generate assembly for a file
src/fs/FileData.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/FILE_DATA.dir/build.make CMakeFiles/FILE_DATA.dir/src/fs/FileData.cpp.s
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/src/fs/FileData.cpp.s
.PHONY : src/fs/FileData.cpp.s

src/fs/FileSystem.o: src/fs/FileSystem.cpp.o

.PHONY : src/fs/FileSystem.o

# target to build an object file
src/fs/FileSystem.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/FILE_SYSTEM.dir/build.make CMakeFiles/FILE_SYSTEM.dir/src/fs/FileSystem.cpp.o
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/src/fs/FileSystem.cpp.o
.PHONY : src/fs/FileSystem.cpp.o

src/fs/FileSystem.i: src/fs/FileSystem.cpp.i

.PHONY : src/fs/FileSystem.i

# target to preprocess a source file
src/fs/FileSystem.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/FILE_SYSTEM.dir/build.make CMakeFiles/FILE_SYSTEM.dir/src/fs/FileSystem.cpp.i
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/src/fs/FileSystem.cpp.i
.PHONY : src/fs/FileSystem.cpp.i

src/fs/FileSystem.s: src/fs/FileSystem.cpp.s

.PHONY : src/fs/FileSystem.s

# target to generate assembly for a file
src/fs/FileSystem.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/FILE_SYSTEM.dir/build.make CMakeFiles/FILE_SYSTEM.dir/src/fs/FileSystem.cpp.s
	$(MAKE) $(MAKESILENT) -f CMakeFiles/primitive_fs.dir/build.make CMakeFiles/primitive_fs.dir/src/fs/FileSystem.cpp.s
.PHONY : src/fs/FileSystem.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... FILE_DATA"
	@echo "... FILE_SYSTEM"
	@echo "... FUNCTIONS"
	@echo "... primitive_fs"
	@echo "... src/app/PrimitiveFsApp.o"
	@echo "... src/app/PrimitiveFsApp.i"
	@echo "... src/app/PrimitiveFsApp.s"
	@echo "... src/app/main.o"
	@echo "... src/app/main.i"
	@echo "... src/app/main.s"
	@echo "... src/command/function.o"
	@echo "... src/command/function.i"
	@echo "... src/command/function.s"
	@echo "... src/common/structures.o"
	@echo "... src/common/structures.i"
	@echo "... src/common/structures.s"
	@echo "... src/fs/FileData.o"
	@echo "... src/fs/FileData.i"
	@echo "... src/fs/FileData.s"
	@echo "... src/fs/FileSystem.o"
	@echo "... src/fs/FileSystem.i"
	@echo "... src/fs/FileSystem.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

