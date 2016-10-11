########### module: kernel #########
cmake_minimum_required (VERSION 3.3)

project (UtopiaOS_kernel VERSION 0.1)

# Debug build system
set (CMAKE_VERBOSE_MAKEFILE ON)

# Set the c++ standard that is required!
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (MODULE_KERNEL_SOURCES kernel_main.cpp)

if (UTOPIAOS_HOSTED)
  add_library (kernel SHARED ${MODULE_KERNEL_SOURCES})
  set_target_properties (kernel PROPERTIES TARGET_TYPE library)
else ()
  add_executable (kernel ${MODULE_KERNEL_SOURCES})
  set_target_properties (kernel PROPERTIES TARGET_TYPE executable)
endif ()

# Append to target lists
list (APPEND UTOPIAOS_SUBTARGETS kernel)

# Propagate upwards
set (UTOPIAOS_SUBTARGETS ${UTOPIAOS_SUBTARGETS} PARENT_SCOPE)