########### module: environment #########
cmake_minimum_required (VERSION 3.3)

project (UtopiaOS_environment VERSION 0.1.0.0)

set (MODULE_ENVIRONMENT_CONFIG ${UtopiaOS_BINARY_DIR}/include/environment/project_config.hpp)

configure_file (project_config.cmake ${MODULE_ENVIRONMENT_CONFIG})

set (MODULE_ENVIRONMENT_HEADERS
  ${MODULE_ENVIRONMENT_CONFIG}
  ${CMAKE_CURRENT_SOURCE_DIR}/environment.hpp
)
set (MODULE_ENVIRONMENT_SOURCES)

add_library (environment INTERFACE)
target_sources (environment INTERFACE
  ${MODULE_ENVIRONMENT_HEADERS}
  ${MODULE_ENVIRONMENT_SOURCES}
)

target_link_libraries (environment INTERFACE target UEFI utils)