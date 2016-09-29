########### module: kernel #########

# I do not really know what minimum version is needed…
cmake_minimum_required (VERSION 3.0)
 
# The modules project version number.
set (MODULE_KERNEL_VERSION_MAJOR 0)
set (MODULE_KERNEL_VERSION_MINOR 1)

# The bookkeeping
set(MODULE_KERNEL_SOURCES kernel_main.cpp)

# The kernel consists of exactly one executable
add_executable(kernel ${MODULE_KERNEL_SOURCES})
