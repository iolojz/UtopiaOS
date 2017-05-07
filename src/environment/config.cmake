#ifndef H_UtopiaOS_environment_config
#define H_UtopiaOS_environment_config

#include "../config.hpp"

#define UTOPIAOS_ENVIRONMENT_VERSION_MAJOR @UtopiaOS_environment_VERSION_MAJOR@
#define UTOPIAOS_ENVIRONMENT_VERSION_MINOR @UtopiaOS_environment_VERSION_MINOR@
#define UTOPIAOS_ENVIRONMENT_VERSION_PATCH @UtopiaOS_environment_VERSION_PATCH@
#define UTOPIAOS_ENVIRONMENT_VERSION_TWEAK @UtopiaOS_environment_VERSION_TWEAK@

#define UTOPIAOS_ENVIRONMENT_VERSION_HUMAN @UtopiaOS_environment_VERSION@
#define UTOPIAOS_ENVIRONMENT_VERSION_CXX @UtopiaOS_environment_VERSION_MAJOR@ ## x ## \
                                         @UtopiaOS_environment_VERSION_MINOR@ ## x ## \
                                         @UtopiaOS_environment_VERSION_PATCH@ ## x ## \
                                         @UtopiaOS_environment_VERSION_TWEAK@

#endif
