########### module: target #########
cmake_minimum_required (VERSION 3.3)

project (UtopiaOS_target VERSION 0.1.0.0)

set (MODULE_TARGET_CONFIG ${UtopiaOS_BINARY_DIR}/include/target/config.hpp)

configure_file (config.cmake ${MODULE_TARGET_CONFIG})

set (MODULE_TARGET_HEADERS
  ${MODULE_TARGET_CONFIG}
  ${CMAKE_CURRENT_SOURCE_DIR}/target.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/memory.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/UEFITypes.hpp
)
set (MODULE_TARGET_SOURCES)

add_library (target INTERFACE)
target_sources (target INTERFACE
  ${MODULE_TARGET_HEADERS}
  ${MODULE_TARGET_SOURCES}
)
