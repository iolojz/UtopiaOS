########### module: UEFI #########
cmake_minimum_required (VERSION 3.3)

project (UtopiaOS_UEFI VERSION 0.1.0.0)

set (MODULE_UEFI_CONFIG ${UtopiaOS_BINARY_DIR}/include/UEFI/config.hpp)

configure_file (config.cmake ${MODULE_UEFI_CONFIG})

set (MODULE_UEFI_HEADERS
  ${MODULE_UEFI_CONFIG}
  ${CMAKE_CURRENT_SOURCE_DIR}/memory.hpp
)
set (MODULE_UEFI_SOURCES)

add_library (UEFI INTERFACE)
target_sources (UEFI INTERFACE
  ${MODULE_UEFI_HEADERS}
  ${MODULE_UEFI_SOURCES}
)

target_link_libraries (UEFI INTERFACE target)
