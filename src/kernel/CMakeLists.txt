########### module: kernel #########
cmake_minimum_required (VERSION 3.3)

project (UtopiaOS_kernel VERSION 0.1.0.0)
configure_file (project_config.cmake ${UtopiaOS_BINARY_DIR}/include/kernel/project_config.hpp)

set (MODULE_KERNEL_SOURCES kernel_main.cpp buddy_resource.cpp)

if (UTOPIAOS_HOSTED)
  add_library (kernel SHARED ${MODULE_KERNEL_SOURCES})
  set_target_properties (kernel PROPERTIES TARGET_TYPE library)
else ()
  add_executable (kernel ${MODULE_KERNEL_SOURCES})
  set_target_properties (kernel PROPERTIES TARGET_TYPE executable)
endif ()
