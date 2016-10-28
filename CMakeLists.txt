############## UtopiaOS ############
cmake_minimum_required (VERSION 3.3)

project (UtopiaOS VERSION 0.1)

set (CMAKE_VERBOSE_MAKEFILE YES)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

# Set the c++ standard that is required!
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS NO)

find_package (Boost 1.6 REQUIRED)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

if (${CMAKE_CROSSCOMPILING})
  set (UTOPIAOS_HOSTED NO)
else ()
  set (UTOPIAOS_HOSTED YES)
endif ()

set (UTOPIAOS_SUBTARGETS) # Will be filled by subdirectories

add_subdirectory (src)

#foreach (subtarget ${UTOPIAOS_SUBTARGETS})
#  get_target_property (subtarget_type ${subtarget} TARGET_TYPE)
#  
#  if (${subtarget_type} STREQUAL executable)
#    add_executable (${subtarget} IMPORTED)
#  elseif (${subtarget_type} STREQUAL library)
#    add_library (${subtarget} IMPORTED)
#  else ()
#    message (FATAL_ERROR "unsupported subtarget type ${subtarget_type}")
#  endif ()
#endforeach (subtarget)
