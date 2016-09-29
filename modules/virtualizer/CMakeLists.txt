############## virtualizer ############

# I do not really know what minimum version is needed…
cmake_minimum_required (VERSION 3.0)

set (VIRTUALIZER_VERSION_MAJOR 0)
set (VIRTUALIZER_BOOT_VERSION_MINOR 1)

set(VIRTUALIZER_SOURCES main.cpp)
set(VIRTUALIZER_LIBS ${LIBS})

add_executable(virtualizer ${VIRTUALIZER_SOURCES})
target_link_libraries(virtualizer ${VIRTUALIZER_LIBS})
