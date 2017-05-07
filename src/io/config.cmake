#ifndef H_UtopiaOS_environment_config
#define H_UtopiaOS_environment_config

#include "../config.hpp"

#define UTOPIAOS_IO_VERSION_MAJOR @UtopiaOS_io_VERSION_MAJOR@
#define UTOPIAOS_IO_VERSION_MINOR @UtopiaOS_io_VERSION_MINOR@
#define UTOPIAOS_IO_VERSION_PATCH @UtopiaOS_io_VERSION_PATCH@
#define UTOPIAOS_IO_VERSION_TWEAK @UtopiaOS_io_VERSION_TWEAK@

#define UTOPIAOS_IO_VERSION_HUMAN @UtopiaOS_io_VERSION@
#define UTOPIAOS_IO_VERSION_CXX @UtopiaOS_io_VERSION_MAJOR@ ## x ## \
                                @UtopiaOS_io_VERSION_MINOR@ ## x ## \
                                @UtopiaOS_io_VERSION_PATCH@ ## x ## \
                                @UtopiaOS_io_VERSION_TWEAK@

#endif
