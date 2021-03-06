########### module: kernel #########
cmake_minimum_required (VERSION 3.8)

project (UtopiaOS_kernel VERSION 0.1.0.0)

set (MODULE_KERNEL_CONFIG ${UtopiaOS_BINARY_DIR}/include/kernel/config.hpp)

configure_file (config.cmake ${MODULE_KERNEL_CONFIG})

set (MODULE_KERNEL_HEADERS
  ${MODULE_KERNEL_CONFIG}
  constants.hpp
  distributed_resource.hpp
  buddy_resource.hpp
  kernel_main.hpp
  memory_manager.hpp
  memory_map.hpp
)
set (MODULE_KERNEL_SOURCES
  buddy_resource.cpp
  kernel_main.cpp
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

target_link_libraries (kernel io environment UEFI utils target)
