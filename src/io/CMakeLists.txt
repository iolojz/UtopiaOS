########### module: io #########
cmake_minimum_required (VERSION 3.3)

project (UtopiaOS_io VERSION 0.1.0.0)
configure_file (project_config.cmake ${UtopiaOS_BINARY_DIR}/include/io/project_config.hpp)

# Set the c++ standard that is required!
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (MODULE_KERNEL_SOURCES logger.cpp)

add_library (io SHARED ${MODULE_KERNEL_SOURCES})
set_target_properties (io PROPERTIES TARGET_TYPE library)
