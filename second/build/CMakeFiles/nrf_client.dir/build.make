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
CMAKE_SOURCE_DIR = /mnt/hgfs/ubuntushare/nrf24l01_4418n/second

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/build

# Include any dependencies generated for this target.
include CMakeFiles/nrf_client.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/nrf_client.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/nrf_client.dir/flags.make

CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.o: CMakeFiles/nrf_client.dir/flags.make
CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.o: ../src/nrf24l01.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/ubuntushare/nrf24l01_4418n/second/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.o"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.o -c /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/src/nrf24l01.cpp

CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.i"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/src/nrf24l01.cpp > CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.i

CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.s"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/src/nrf24l01.cpp -o CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.s

CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.o.requires:

.PHONY : CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.o.requires

CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.o.provides: CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.o.requires
	$(MAKE) -f CMakeFiles/nrf_client.dir/build.make CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.o.provides.build
.PHONY : CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.o.provides

CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.o.provides.build: CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.o


CMakeFiles/nrf_client.dir/src/list.cpp.o: CMakeFiles/nrf_client.dir/flags.make
CMakeFiles/nrf_client.dir/src/list.cpp.o: ../src/list.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/ubuntushare/nrf24l01_4418n/second/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/nrf_client.dir/src/list.cpp.o"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nrf_client.dir/src/list.cpp.o -c /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/src/list.cpp

CMakeFiles/nrf_client.dir/src/list.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nrf_client.dir/src/list.cpp.i"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/src/list.cpp > CMakeFiles/nrf_client.dir/src/list.cpp.i

CMakeFiles/nrf_client.dir/src/list.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nrf_client.dir/src/list.cpp.s"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/src/list.cpp -o CMakeFiles/nrf_client.dir/src/list.cpp.s

CMakeFiles/nrf_client.dir/src/list.cpp.o.requires:

.PHONY : CMakeFiles/nrf_client.dir/src/list.cpp.o.requires

CMakeFiles/nrf_client.dir/src/list.cpp.o.provides: CMakeFiles/nrf_client.dir/src/list.cpp.o.requires
	$(MAKE) -f CMakeFiles/nrf_client.dir/build.make CMakeFiles/nrf_client.dir/src/list.cpp.o.provides.build
.PHONY : CMakeFiles/nrf_client.dir/src/list.cpp.o.provides

CMakeFiles/nrf_client.dir/src/list.cpp.o.provides.build: CMakeFiles/nrf_client.dir/src/list.cpp.o


CMakeFiles/nrf_client.dir/src/mytime.cpp.o: CMakeFiles/nrf_client.dir/flags.make
CMakeFiles/nrf_client.dir/src/mytime.cpp.o: ../src/mytime.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/ubuntushare/nrf24l01_4418n/second/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/nrf_client.dir/src/mytime.cpp.o"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nrf_client.dir/src/mytime.cpp.o -c /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/src/mytime.cpp

CMakeFiles/nrf_client.dir/src/mytime.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nrf_client.dir/src/mytime.cpp.i"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/src/mytime.cpp > CMakeFiles/nrf_client.dir/src/mytime.cpp.i

CMakeFiles/nrf_client.dir/src/mytime.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nrf_client.dir/src/mytime.cpp.s"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/src/mytime.cpp -o CMakeFiles/nrf_client.dir/src/mytime.cpp.s

CMakeFiles/nrf_client.dir/src/mytime.cpp.o.requires:

.PHONY : CMakeFiles/nrf_client.dir/src/mytime.cpp.o.requires

CMakeFiles/nrf_client.dir/src/mytime.cpp.o.provides: CMakeFiles/nrf_client.dir/src/mytime.cpp.o.requires
	$(MAKE) -f CMakeFiles/nrf_client.dir/build.make CMakeFiles/nrf_client.dir/src/mytime.cpp.o.provides.build
.PHONY : CMakeFiles/nrf_client.dir/src/mytime.cpp.o.provides

CMakeFiles/nrf_client.dir/src/mytime.cpp.o.provides.build: CMakeFiles/nrf_client.dir/src/mytime.cpp.o


CMakeFiles/nrf_client.dir/src/pack.cpp.o: CMakeFiles/nrf_client.dir/flags.make
CMakeFiles/nrf_client.dir/src/pack.cpp.o: ../src/pack.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/ubuntushare/nrf24l01_4418n/second/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/nrf_client.dir/src/pack.cpp.o"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nrf_client.dir/src/pack.cpp.o -c /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/src/pack.cpp

CMakeFiles/nrf_client.dir/src/pack.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nrf_client.dir/src/pack.cpp.i"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/src/pack.cpp > CMakeFiles/nrf_client.dir/src/pack.cpp.i

CMakeFiles/nrf_client.dir/src/pack.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nrf_client.dir/src/pack.cpp.s"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/src/pack.cpp -o CMakeFiles/nrf_client.dir/src/pack.cpp.s

CMakeFiles/nrf_client.dir/src/pack.cpp.o.requires:

.PHONY : CMakeFiles/nrf_client.dir/src/pack.cpp.o.requires

CMakeFiles/nrf_client.dir/src/pack.cpp.o.provides: CMakeFiles/nrf_client.dir/src/pack.cpp.o.requires
	$(MAKE) -f CMakeFiles/nrf_client.dir/build.make CMakeFiles/nrf_client.dir/src/pack.cpp.o.provides.build
.PHONY : CMakeFiles/nrf_client.dir/src/pack.cpp.o.provides

CMakeFiles/nrf_client.dir/src/pack.cpp.o.provides.build: CMakeFiles/nrf_client.dir/src/pack.cpp.o


CMakeFiles/nrf_client.dir/src/nrf_client.cpp.o: CMakeFiles/nrf_client.dir/flags.make
CMakeFiles/nrf_client.dir/src/nrf_client.cpp.o: ../src/nrf_client.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/ubuntushare/nrf24l01_4418n/second/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/nrf_client.dir/src/nrf_client.cpp.o"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-g++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nrf_client.dir/src/nrf_client.cpp.o -c /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/src/nrf_client.cpp

CMakeFiles/nrf_client.dir/src/nrf_client.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nrf_client.dir/src/nrf_client.cpp.i"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/src/nrf_client.cpp > CMakeFiles/nrf_client.dir/src/nrf_client.cpp.i

CMakeFiles/nrf_client.dir/src/nrf_client.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nrf_client.dir/src/nrf_client.cpp.s"
	/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/src/nrf_client.cpp -o CMakeFiles/nrf_client.dir/src/nrf_client.cpp.s

CMakeFiles/nrf_client.dir/src/nrf_client.cpp.o.requires:

.PHONY : CMakeFiles/nrf_client.dir/src/nrf_client.cpp.o.requires

CMakeFiles/nrf_client.dir/src/nrf_client.cpp.o.provides: CMakeFiles/nrf_client.dir/src/nrf_client.cpp.o.requires
	$(MAKE) -f CMakeFiles/nrf_client.dir/build.make CMakeFiles/nrf_client.dir/src/nrf_client.cpp.o.provides.build
.PHONY : CMakeFiles/nrf_client.dir/src/nrf_client.cpp.o.provides

CMakeFiles/nrf_client.dir/src/nrf_client.cpp.o.provides.build: CMakeFiles/nrf_client.dir/src/nrf_client.cpp.o


# Object files for target nrf_client
nrf_client_OBJECTS = \
"CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.o" \
"CMakeFiles/nrf_client.dir/src/list.cpp.o" \
"CMakeFiles/nrf_client.dir/src/mytime.cpp.o" \
"CMakeFiles/nrf_client.dir/src/pack.cpp.o" \
"CMakeFiles/nrf_client.dir/src/nrf_client.cpp.o"

# External object files for target nrf_client
nrf_client_EXTERNAL_OBJECTS =

../bin/nrf_client: CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.o
../bin/nrf_client: CMakeFiles/nrf_client.dir/src/list.cpp.o
../bin/nrf_client: CMakeFiles/nrf_client.dir/src/mytime.cpp.o
../bin/nrf_client: CMakeFiles/nrf_client.dir/src/pack.cpp.o
../bin/nrf_client: CMakeFiles/nrf_client.dir/src/nrf_client.cpp.o
../bin/nrf_client: CMakeFiles/nrf_client.dir/build.make
../bin/nrf_client: CMakeFiles/nrf_client.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/hgfs/ubuntushare/nrf24l01_4418n/second/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX executable ../bin/nrf_client"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/nrf_client.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/nrf_client.dir/build: ../bin/nrf_client

.PHONY : CMakeFiles/nrf_client.dir/build

CMakeFiles/nrf_client.dir/requires: CMakeFiles/nrf_client.dir/src/nrf24l01.cpp.o.requires
CMakeFiles/nrf_client.dir/requires: CMakeFiles/nrf_client.dir/src/list.cpp.o.requires
CMakeFiles/nrf_client.dir/requires: CMakeFiles/nrf_client.dir/src/mytime.cpp.o.requires
CMakeFiles/nrf_client.dir/requires: CMakeFiles/nrf_client.dir/src/pack.cpp.o.requires
CMakeFiles/nrf_client.dir/requires: CMakeFiles/nrf_client.dir/src/nrf_client.cpp.o.requires

.PHONY : CMakeFiles/nrf_client.dir/requires

CMakeFiles/nrf_client.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/nrf_client.dir/cmake_clean.cmake
.PHONY : CMakeFiles/nrf_client.dir/clean

CMakeFiles/nrf_client.dir/depend:
	cd /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/hgfs/ubuntushare/nrf24l01_4418n/second /mnt/hgfs/ubuntushare/nrf24l01_4418n/second /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/build /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/build /mnt/hgfs/ubuntushare/nrf24l01_4418n/second/build/CMakeFiles/nrf_client.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/nrf_client.dir/depend

