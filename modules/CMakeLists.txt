########### modules #########

# I do not really know what minimum version is needed…
cmake_minimum_required (VERSION 3.0)
 
# The modules project version number.
set (MODULES_VERSION_MAJOR 0)
set (MODULES_VERSION_MINOR 1)

# The list of modules
set(MODULES kernel)

foreach(module ${MODULES})
  add_subdirectory(${module})
endforeach(module)
