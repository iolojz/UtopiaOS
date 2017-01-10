########### module: utils #########
cmake_minimum_required (VERSION 3.3)

project (UtopiaOS_utils VERSION 0.1.0.0)

set (MODULE_UTILS_CONFIG ${UtopiaOS_BINARY_DIR}/include/utils/project_config.hpp)

configure_file (project_config.cmake ${MODULE_UTILS_CONFIG})

set (MODULE_UTILS_HEADERS
  ${MODULE_UTILS_CONFIG}
  assert.hpp
  bitwise.hpp
  constructor.hpp
  debug.hpp
  destruct_deleter.hpp
  dynarray.hpp
  make_array.hpp
  ranges.hpp
  trap.hpp
)
set (MODULE_UTILS_SOURCES)

add_library (utils INTERFACE)
target_sources (utils INTERFACE
  ${MODULE_UTILS_HEADERS}
  ${MODULE_UTILS_SOURCES}
)

add_dependencies (utils target io)
