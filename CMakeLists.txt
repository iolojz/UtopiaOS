############## UtopiaOS ############
cmake_minimum_required (VERSION 3.8)

project (UtopiaOS VERSION 0.1.0.0)

set (CMAKE_VERBOSE_MAKEFILE YES)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

# Set the c++ standard that is required!
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS NO)

find_package (Boost 1.6 REQUIRED)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

if (${CMAKE_CROSSCOMPILING})
  set (UTOPIAOS_HOSTED NO)
  set (UTOPIAOS_HOSTED_ASNUMBER 0)
else ()
  set (UTOPIAOS_HOSTED YES)
  set (UTOPIAOS_HOSTED_ASNUMBER 1)
endif ()

configure_file (config.cmake include/config.hpp)
include_directories (${PROJECT_BINARY_DIR}/include)

add_subdirectory (src)
