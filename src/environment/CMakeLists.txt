########### module: environment #########
cmake_minimum_required (VERSION 3.3)

project (UtopiaOS_environment VERSION 0.1.0.0)

configure_file (project_config.cmake ${UtopiaOS_BINARY_DIR}/include/environment/project_config.hpp)
