########### module: utils #########
cmake_minimum_required (VERSION 3.3)

project (UtopiaOS_utils VERSION 0.1.0.0)

set (MODULE_UTILS_CONFIG ${UtopiaOS_BINARY_DIR}/include/utils/project_config.hpp)

configure_file (project_config.cmake ${MODULE_UTILS_CONFIG})

set (MODULE_UTILS_HEADERS
  ${MODULE_UTILS_CONFIG}
  ${CMAKE_CURRENT_SOURCE_DIR}/assert.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/bitwise.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/constructor.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/debug.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/destruct_deleter.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/dynarray.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/make_array.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ranges.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/trap.hpp
)
set (MODULE_UTILS_SOURCES)

add_library (utils INTERFACE)
target_sources (utils INTERFACE
  ${MODULE_UTILS_HEADERS}
  ${MODULE_UTILS_SOURCES}
)

target_link_libraries (utils INTERFACE io target)
