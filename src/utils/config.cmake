#ifndef H_UtopiaOS_utils_config
#define H_UtopiaOS_utils_config

#include "../config.hpp"

#define UTOPIAOS_UTILS_VERSION_MAJOR @UtopiaOS_utils_VERSION_MAJOR@
#define UTOPIAOS_UTILS_VERSION_MINOR @UtopiaOS_utils_VERSION_MINOR@
#define UTOPIAOS_UTILS_VERSION_PATCH @UtopiaOS_utils_VERSION_PATCH@
#define UTOPIAOS_UTILS_VERSION_TWEAK @UtopiaOS_utils_VERSION_TWEAK@

#define UTOPIAOS_UTILS_VERSION_HUMAN @UtopiaOS_utils_VERSION@
#define UTOPIAOS_UTILS_VERSION_CXX @UtopiaOS_utils_VERSION_MAJOR@ ## x ## \
                                   @UtopiaOS_utils_VERSION_MINOR@ ## x ## \
                                   @UtopiaOS_utils_VERSION_PATCH@ ## x ## \
                                   @UtopiaOS_utils_VERSION_TWEAK@

#endif
