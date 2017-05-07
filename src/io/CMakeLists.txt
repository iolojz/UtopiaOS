########### module: io #########
cmake_minimum_required (VERSION 3.3)

project (UtopiaOS_io VERSION 0.1.0.0)

set (MODULE_IO_CONFIG ${UtopiaOS_BINARY_DIR}/include/io/config.hpp)

configure_file (config.cmake ${MODULE_IO_CONFIG})

set (MODULE_IO_HEADERS
  ${MODULE_IO_CONFIG}
  logger.hpp
  string.hpp
)
set (MODULE_IO_SOURCES
  logger.cpp
)

add_library (io SHARED
  ${MODULE_IO_HEADERS}
  ${MODULE_IO_SOURCES}
)

target_link_libraries (io target)
