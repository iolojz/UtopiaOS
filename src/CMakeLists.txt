########### modules #########
cmake_minimum_required (VERSION 3.3)

include_directories (SYSTEM .)

# The list of modules
set (MODULES environment io kernel target UEFI utils)

foreach(module ${MODULES})
  add_subdirectory (${module})
endforeach(module)

# Propagate upwards
set (UTOPIAOS_SUBTARGETS ${UTOPIAOS_SUBTARGETS} PARENT_SCOPE)