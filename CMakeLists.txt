############## UtopiaOS ############
cmake_minimum_required (VERSION 3.3)

project (UtopiaOS VERSION 0.1)

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
