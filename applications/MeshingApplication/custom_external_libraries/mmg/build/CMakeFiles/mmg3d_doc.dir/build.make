# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

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
CMAKE_SOURCE_DIR = /home/mhashemi/programming/Git/Kratos-working/applications/MeshingApplication/custom_external_libraries/mmg

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/mhashemi/programming/Git/Kratos-working/applications/MeshingApplication/custom_external_libraries/mmg/build

# Utility rule file for mmg3d_doc.

# Include the progress variables for this target.
include CMakeFiles/mmg3d_doc.dir/progress.make

CMakeFiles/mmg3d_doc:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/mhashemi/programming/Git/Kratos-working/applications/MeshingApplication/custom_external_libraries/mmg/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating MMG3D API documentation with Doxygen. Open up the"
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold "    /home/mhashemi/programming/Git/Kratos-working/applications/MeshingApplication/custom_external_libraries/mmg/doc/doxygen/mmg3d/html/index.html file to see"
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold "    it"
	cd /home/mhashemi/programming/Git/Kratos-working/applications/MeshingApplication/custom_external_libraries/mmg/build/doc/mmg3d && /usr/bin/doxygen /home/mhashemi/programming/Git/Kratos-working/applications/MeshingApplication/custom_external_libraries/mmg/doc/doxygen/mmg3d/Doxyfile

mmg3d_doc: CMakeFiles/mmg3d_doc
mmg3d_doc: CMakeFiles/mmg3d_doc.dir/build.make

.PHONY : mmg3d_doc

# Rule to build all files generated by this target.
CMakeFiles/mmg3d_doc.dir/build: mmg3d_doc

.PHONY : CMakeFiles/mmg3d_doc.dir/build

CMakeFiles/mmg3d_doc.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/mmg3d_doc.dir/cmake_clean.cmake
.PHONY : CMakeFiles/mmg3d_doc.dir/clean

CMakeFiles/mmg3d_doc.dir/depend:
	cd /home/mhashemi/programming/Git/Kratos-working/applications/MeshingApplication/custom_external_libraries/mmg/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mhashemi/programming/Git/Kratos-working/applications/MeshingApplication/custom_external_libraries/mmg /home/mhashemi/programming/Git/Kratos-working/applications/MeshingApplication/custom_external_libraries/mmg /home/mhashemi/programming/Git/Kratos-working/applications/MeshingApplication/custom_external_libraries/mmg/build /home/mhashemi/programming/Git/Kratos-working/applications/MeshingApplication/custom_external_libraries/mmg/build /home/mhashemi/programming/Git/Kratos-working/applications/MeshingApplication/custom_external_libraries/mmg/build/CMakeFiles/mmg3d_doc.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/mmg3d_doc.dir/depend
