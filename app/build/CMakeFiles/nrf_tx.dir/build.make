# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/hgfs/ubuntushare/nrf24l01_4418n/app

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/hgfs/ubuntushare/nrf24l01_4418n/app/build

# Include any dependencies generated for this target.
include CMakeFiles/nrf_tx.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/nrf_tx.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/nrf_tx.dir/flags.make

CMakeFiles/nrf_tx.dir/src/nrf24l01.c.o: CMakeFiles/nrf_tx.dir/flags.make
CMakeFiles/nrf_tx.dir/src/nrf24l01.c.o: ../src/nrf24l01.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/ubuntushare/nrf24l01_4418n/app/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/nrf_tx.dir/src/nrf24l01.c.o"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/nrf_tx.dir/src/nrf24l01.c.o   -c /mnt/hgfs/ubuntushare/nrf24l01_4418n/app/src/nrf24l01.c

CMakeFiles/nrf_tx.dir/src/nrf24l01.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/nrf_tx.dir/src/nrf24l01.c.i"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/hgfs/ubuntushare/nrf24l01_4418n/app/src/nrf24l01.c > CMakeFiles/nrf_tx.dir/src/nrf24l01.c.i

CMakeFiles/nrf_tx.dir/src/nrf24l01.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/nrf_tx.dir/src/nrf24l01.c.s"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/hgfs/ubuntushare/nrf24l01_4418n/app/src/nrf24l01.c -o CMakeFiles/nrf_tx.dir/src/nrf24l01.c.s

CMakeFiles/nrf_tx.dir/src/nrf24l01.c.o.requires:

.PHONY : CMakeFiles/nrf_tx.dir/src/nrf24l01.c.o.requires

CMakeFiles/nrf_tx.dir/src/nrf24l01.c.o.provides: CMakeFiles/nrf_tx.dir/src/nrf24l01.c.o.requires
	$(MAKE) -f CMakeFiles/nrf_tx.dir/build.make CMakeFiles/nrf_tx.dir/src/nrf24l01.c.o.provides.build
.PHONY : CMakeFiles/nrf_tx.dir/src/nrf24l01.c.o.provides

CMakeFiles/nrf_tx.dir/src/nrf24l01.c.o.provides.build: CMakeFiles/nrf_tx.dir/src/nrf24l01.c.o


CMakeFiles/nrf_tx.dir/src/list.c.o: CMakeFiles/nrf_tx.dir/flags.make
CMakeFiles/nrf_tx.dir/src/list.c.o: ../src/list.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/ubuntushare/nrf24l01_4418n/app/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/nrf_tx.dir/src/list.c.o"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/nrf_tx.dir/src/list.c.o   -c /mnt/hgfs/ubuntushare/nrf24l01_4418n/app/src/list.c

CMakeFiles/nrf_tx.dir/src/list.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/nrf_tx.dir/src/list.c.i"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/hgfs/ubuntushare/nrf24l01_4418n/app/src/list.c > CMakeFiles/nrf_tx.dir/src/list.c.i

CMakeFiles/nrf_tx.dir/src/list.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/nrf_tx.dir/src/list.c.s"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/hgfs/ubuntushare/nrf24l01_4418n/app/src/list.c -o CMakeFiles/nrf_tx.dir/src/list.c.s

CMakeFiles/nrf_tx.dir/src/list.c.o.requires:

.PHONY : CMakeFiles/nrf_tx.dir/src/list.c.o.requires

CMakeFiles/nrf_tx.dir/src/list.c.o.provides: CMakeFiles/nrf_tx.dir/src/list.c.o.requires
	$(MAKE) -f CMakeFiles/nrf_tx.dir/build.make CMakeFiles/nrf_tx.dir/src/list.c.o.provides.build
.PHONY : CMakeFiles/nrf_tx.dir/src/list.c.o.provides

CMakeFiles/nrf_tx.dir/src/list.c.o.provides.build: CMakeFiles/nrf_tx.dir/src/list.c.o


CMakeFiles/nrf_tx.dir/src/nrf_tx.c.o: CMakeFiles/nrf_tx.dir/flags.make
CMakeFiles/nrf_tx.dir/src/nrf_tx.c.o: ../src/nrf_tx.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/ubuntushare/nrf24l01_4418n/app/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/nrf_tx.dir/src/nrf_tx.c.o"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/nrf_tx.dir/src/nrf_tx.c.o   -c /mnt/hgfs/ubuntushare/nrf24l01_4418n/app/src/nrf_tx.c

CMakeFiles/nrf_tx.dir/src/nrf_tx.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/nrf_tx.dir/src/nrf_tx.c.i"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/hgfs/ubuntushare/nrf24l01_4418n/app/src/nrf_tx.c > CMakeFiles/nrf_tx.dir/src/nrf_tx.c.i

CMakeFiles/nrf_tx.dir/src/nrf_tx.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/nrf_tx.dir/src/nrf_tx.c.s"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-gcc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/hgfs/ubuntushare/nrf24l01_4418n/app/src/nrf_tx.c -o CMakeFiles/nrf_tx.dir/src/nrf_tx.c.s

CMakeFiles/nrf_tx.dir/src/nrf_tx.c.o.requires:

.PHONY : CMakeFiles/nrf_tx.dir/src/nrf_tx.c.o.requires

CMakeFiles/nrf_tx.dir/src/nrf_tx.c.o.provides: CMakeFiles/nrf_tx.dir/src/nrf_tx.c.o.requires
	$(MAKE) -f CMakeFiles/nrf_tx.dir/build.make CMakeFiles/nrf_tx.dir/src/nrf_tx.c.o.provides.build
.PHONY : CMakeFiles/nrf_tx.dir/src/nrf_tx.c.o.provides

CMakeFiles/nrf_tx.dir/src/nrf_tx.c.o.provides.build: CMakeFiles/nrf_tx.dir/src/nrf_tx.c.o


# Object files for target nrf_tx
nrf_tx_OBJECTS = \
"CMakeFiles/nrf_tx.dir/src/nrf24l01.c.o" \
"CMakeFiles/nrf_tx.dir/src/list.c.o" \
"CMakeFiles/nrf_tx.dir/src/nrf_tx.c.o"

# External object files for target nrf_tx
nrf_tx_EXTERNAL_OBJECTS =

../bin/nrf_tx: CMakeFiles/nrf_tx.dir/src/nrf24l01.c.o
../bin/nrf_tx: CMakeFiles/nrf_tx.dir/src/list.c.o
../bin/nrf_tx: CMakeFiles/nrf_tx.dir/src/nrf_tx.c.o
../bin/nrf_tx: CMakeFiles/nrf_tx.dir/build.make
../bin/nrf_tx: CMakeFiles/nrf_tx.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/hgfs/ubuntushare/nrf24l01_4418n/app/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C executable ../bin/nrf_tx"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/nrf_tx.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/nrf_tx.dir/build: ../bin/nrf_tx

.PHONY : CMakeFiles/nrf_tx.dir/build

CMakeFiles/nrf_tx.dir/requires: CMakeFiles/nrf_tx.dir/src/nrf24l01.c.o.requires
CMakeFiles/nrf_tx.dir/requires: CMakeFiles/nrf_tx.dir/src/list.c.o.requires
CMakeFiles/nrf_tx.dir/requires: CMakeFiles/nrf_tx.dir/src/nrf_tx.c.o.requires

.PHONY : CMakeFiles/nrf_tx.dir/requires

CMakeFiles/nrf_tx.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/nrf_tx.dir/cmake_clean.cmake
.PHONY : CMakeFiles/nrf_tx.dir/clean

CMakeFiles/nrf_tx.dir/depend:
	cd /mnt/hgfs/ubuntushare/nrf24l01_4418n/app/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/hgfs/ubuntushare/nrf24l01_4418n/app /mnt/hgfs/ubuntushare/nrf24l01_4418n/app /mnt/hgfs/ubuntushare/nrf24l01_4418n/app/build /mnt/hgfs/ubuntushare/nrf24l01_4418n/app/build /mnt/hgfs/ubuntushare/nrf24l01_4418n/app/build/CMakeFiles/nrf_tx.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/nrf_tx.dir/depend
