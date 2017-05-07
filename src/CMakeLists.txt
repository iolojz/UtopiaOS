########### modules #########
cmake_minimum_required (VERSION 3.3)

include_directories (.)

# The list of modules
set (MODULES environment io kernel target UEFI utils)

foreach(module ${MODULES})
  add_subdirectory (${module})
endforeach(module)
