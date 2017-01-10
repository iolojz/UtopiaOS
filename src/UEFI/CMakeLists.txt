########### module: UEFI #########
cmake_minimum_required (VERSION 3.3)

project (UtopiaOS_UEFI VERSION 0.1.0.0)

set (MODULE_UEFI_CONFIG ${UtopiaOS_BINARY_DIR}/include/UEFI/project_config.hpp)

configure_file (project_config.cmake ${MODULE_UEFI_CONFIG})

set (MODULE_UEFI_HEADERS
  ${MODULE_UEFI_CONFIG}
  memory.hpp
)
set (MODULE_UEFI_SOURCES)

add_library (UEFI INTERFACE)
target_sources (UEFI INTERFACE
  ${MODULE_UEFI_HEADERS}
  ${MODULE_UEFI_SOURCES}
)

add_dependencies (UEFI target)
