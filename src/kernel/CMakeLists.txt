########### module: kernel #########
cmake_minimum_required (VERSION 3.3)

project (UtopiaOS_kernel VERSION 0.1)

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