# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


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
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.13.4/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.13.4/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/howllow/SharedWithUB/simulator

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/howllow/SharedWithUB/simulator

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/local/Cellar/cmake/3.13.4/bin/cmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake cache editor..."
	/usr/local/Cellar/cmake/3.13.4/bin/ccmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /Users/howllow/SharedWithUB/simulator/CMakeFiles /Users/howllow/SharedWithUB/simulator/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /Users/howllow/SharedWithUB/simulator/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named Simulator

# Build rule for target.
Simulator: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 Simulator
.PHONY : Simulator

# fast build rule for target.
Simulator/fast:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/build
.PHONY : Simulator/fast

src-codes/MM.o: src-codes/MM.cc.o

.PHONY : src-codes/MM.o

# target to build an object file
src-codes/MM.cc.o:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/MM.cc.o
.PHONY : src-codes/MM.cc.o

src-codes/MM.i: src-codes/MM.cc.i

.PHONY : src-codes/MM.i

# target to preprocess a source file
src-codes/MM.cc.i:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/MM.cc.i
.PHONY : src-codes/MM.cc.i

src-codes/MM.s: src-codes/MM.cc.s

.PHONY : src-codes/MM.s

# target to generate assembly for a file
src-codes/MM.cc.s:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/MM.cc.s
.PHONY : src-codes/MM.cc.s

src-codes/Simulator.o: src-codes/Simulator.cc.o

.PHONY : src-codes/Simulator.o

# target to build an object file
src-codes/Simulator.cc.o:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/Simulator.cc.o
.PHONY : src-codes/Simulator.cc.o

src-codes/Simulator.i: src-codes/Simulator.cc.i

.PHONY : src-codes/Simulator.i

# target to preprocess a source file
src-codes/Simulator.cc.i:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/Simulator.cc.i
.PHONY : src-codes/Simulator.cc.i

src-codes/Simulator.s: src-codes/Simulator.cc.s

.PHONY : src-codes/Simulator.s

# target to generate assembly for a file
src-codes/Simulator.cc.s:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/Simulator.cc.s
.PHONY : src-codes/Simulator.cc.s

src-codes/cache.o: src-codes/cache.cc.o

.PHONY : src-codes/cache.o

# target to build an object file
src-codes/cache.cc.o:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/cache.cc.o
.PHONY : src-codes/cache.cc.o

src-codes/cache.i: src-codes/cache.cc.i

.PHONY : src-codes/cache.i

# target to preprocess a source file
src-codes/cache.cc.i:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/cache.cc.i
.PHONY : src-codes/cache.cc.i

src-codes/cache.s: src-codes/cache.cc.s

.PHONY : src-codes/cache.s

# target to generate assembly for a file
src-codes/cache.cc.s:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/cache.cc.s
.PHONY : src-codes/cache.cc.s

src-codes/decode.o: src-codes/decode.cc.o

.PHONY : src-codes/decode.o

# target to build an object file
src-codes/decode.cc.o:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/decode.cc.o
.PHONY : src-codes/decode.cc.o

src-codes/decode.i: src-codes/decode.cc.i

.PHONY : src-codes/decode.i

# target to preprocess a source file
src-codes/decode.cc.i:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/decode.cc.i
.PHONY : src-codes/decode.cc.i

src-codes/decode.s: src-codes/decode.cc.s

.PHONY : src-codes/decode.s

# target to generate assembly for a file
src-codes/decode.cc.s:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/decode.cc.s
.PHONY : src-codes/decode.cc.s

src-codes/execute.o: src-codes/execute.cc.o

.PHONY : src-codes/execute.o

# target to build an object file
src-codes/execute.cc.o:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/execute.cc.o
.PHONY : src-codes/execute.cc.o

src-codes/execute.i: src-codes/execute.cc.i

.PHONY : src-codes/execute.i

# target to preprocess a source file
src-codes/execute.cc.i:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/execute.cc.i
.PHONY : src-codes/execute.cc.i

src-codes/execute.s: src-codes/execute.cc.s

.PHONY : src-codes/execute.s

# target to generate assembly for a file
src-codes/execute.cc.s:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/execute.cc.s
.PHONY : src-codes/execute.cc.s

src-codes/fetch.o: src-codes/fetch.cc.o

.PHONY : src-codes/fetch.o

# target to build an object file
src-codes/fetch.cc.o:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/fetch.cc.o
.PHONY : src-codes/fetch.cc.o

src-codes/fetch.i: src-codes/fetch.cc.i

.PHONY : src-codes/fetch.i

# target to preprocess a source file
src-codes/fetch.cc.i:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/fetch.cc.i
.PHONY : src-codes/fetch.cc.i

src-codes/fetch.s: src-codes/fetch.cc.s

.PHONY : src-codes/fetch.s

# target to generate assembly for a file
src-codes/fetch.cc.s:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/fetch.cc.s
.PHONY : src-codes/fetch.cc.s

src-codes/main.o: src-codes/main.cc.o

.PHONY : src-codes/main.o

# target to build an object file
src-codes/main.cc.o:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/main.cc.o
.PHONY : src-codes/main.cc.o

src-codes/main.i: src-codes/main.cc.i

.PHONY : src-codes/main.i

# target to preprocess a source file
src-codes/main.cc.i:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/main.cc.i
.PHONY : src-codes/main.cc.i

src-codes/main.s: src-codes/main.cc.s

.PHONY : src-codes/main.s

# target to generate assembly for a file
src-codes/main.cc.s:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/main.cc.s
.PHONY : src-codes/main.cc.s

src-codes/memory.o: src-codes/memory.cc.o

.PHONY : src-codes/memory.o

# target to build an object file
src-codes/memory.cc.o:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/memory.cc.o
.PHONY : src-codes/memory.cc.o

src-codes/memory.i: src-codes/memory.cc.i

.PHONY : src-codes/memory.i

# target to preprocess a source file
src-codes/memory.cc.i:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/memory.cc.i
.PHONY : src-codes/memory.cc.i

src-codes/memory.s: src-codes/memory.cc.s

.PHONY : src-codes/memory.s

# target to generate assembly for a file
src-codes/memory.cc.s:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/memory.cc.s
.PHONY : src-codes/memory.cc.s

src-codes/writeBack.o: src-codes/writeBack.cc.o

.PHONY : src-codes/writeBack.o

# target to build an object file
src-codes/writeBack.cc.o:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/writeBack.cc.o
.PHONY : src-codes/writeBack.cc.o

src-codes/writeBack.i: src-codes/writeBack.cc.i

.PHONY : src-codes/writeBack.i

# target to preprocess a source file
src-codes/writeBack.cc.i:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/writeBack.cc.i
.PHONY : src-codes/writeBack.cc.i

src-codes/writeBack.s: src-codes/writeBack.cc.s

.PHONY : src-codes/writeBack.s

# target to generate assembly for a file
src-codes/writeBack.cc.s:
	$(MAKE) -f CMakeFiles/Simulator.dir/build.make CMakeFiles/Simulator.dir/src-codes/writeBack.cc.s
.PHONY : src-codes/writeBack.cc.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... rebuild_cache"
	@echo "... edit_cache"
	@echo "... Simulator"
	@echo "... src-codes/MM.o"
	@echo "... src-codes/MM.i"
	@echo "... src-codes/MM.s"
	@echo "... src-codes/Simulator.o"
	@echo "... src-codes/Simulator.i"
	@echo "... src-codes/Simulator.s"
	@echo "... src-codes/cache.o"
	@echo "... src-codes/cache.i"
	@echo "... src-codes/cache.s"
	@echo "... src-codes/decode.o"
	@echo "... src-codes/decode.i"
	@echo "... src-codes/decode.s"
	@echo "... src-codes/execute.o"
	@echo "... src-codes/execute.i"
	@echo "... src-codes/execute.s"
	@echo "... src-codes/fetch.o"
	@echo "... src-codes/fetch.i"
	@echo "... src-codes/fetch.s"
	@echo "... src-codes/main.o"
	@echo "... src-codes/main.i"
	@echo "... src-codes/main.s"
	@echo "... src-codes/memory.o"
	@echo "... src-codes/memory.i"
	@echo "... src-codes/memory.s"
	@echo "... src-codes/writeBack.o"
	@echo "... src-codes/writeBack.i"
	@echo "... src-codes/writeBack.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

