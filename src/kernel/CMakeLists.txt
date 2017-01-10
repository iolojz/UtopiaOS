########### module: kernel #########
cmake_minimum_required (VERSION 3.3)

project (UtopiaOS_kernel VERSION 0.1.0.0)

set (MODULE_KERNEL_CONFIG ${UtopiaOS_BINARY_DIR}/include/kernel/project_config.hpp)

configure_file (project_config.cmake ${MODULE_KERNEL_CONFIG})

set (MODULE_KERNEL_HEADERS
  ${MODULE_KERNEL_CONFIG}
  buddy_resource.hpp
  constants.hpp
  distributed_resource.hpp
  kernel_main.hpp
  memory_manager.hpp
  memory_map.hpp
)
set (MODULE_KERNEL_SOURCES
  kernel_main.cpp
  buddy_resource.cpp
)

if (UTOPIAOS_HOSTED)
  add_library (kernel SHARED
    ${MODULE_KERNEL_HEADERS}
    ${MODULE_KERNEL_SOURCES}
  )
else ()
  add_executable (kernel
    ${MODULE_KERNEL_HEADERS}
    ${MODULE_KERNEL_SOURCES}
  )
endif ()

add_dependencies (kernel target io environment UEFI utils)
