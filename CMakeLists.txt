############## JayZOS ############

# I do not really know what minimum version is needed…
cmake_minimum_required (VERSION 3.0)
project (OS)
 
# The OS project version number.
set (OS_VERSION_MAJOR 0)
set (OS_VERSION_MINOR 1)

# Everything is a module!
add_subdirectory(modules)

# `convolve` only consists of a single executable file
# add_executable(convolve ${SOURCES})
# set_property(TARGET convolve PROPERTY CXX_STANDARD 11)
# set_property(TARGET convolve PROPERTY CXX_EXTENSIONS OFF)
# target_link_libraries(convolve ${LIBS})
 
# configure a header file to pass some of the CMake settings
# to the source code
# configure_file (
#  "${PROJECT_SOURCE_DIR}/TutorialConfig.h.in"
#  "${PROJECT_BINARY_DIR}/TutorialConfig.h"
#  )

# add the install targets
# install (TARGETS Tutorial DESTINATION bin)
# install (FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h"        
#          DESTINATION include)
 
