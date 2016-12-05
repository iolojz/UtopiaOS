#ifndef H_UtopiaOS_target_project_config
#define H_UtopiaOS_target_project_config

#include "../project_config.hpp"

#define UTOPIAOS_TARGET_VERSION_MAJOR @UtopiaOS_target_VERSION_MAJOR@
#define UTOPIAOS_TARGET_VERSION_MINOR @UtopiaOS_target_VERSION_MINOR@
#define UTOPIAOS_TARGET_VERSION_PATCH @UtopiaOS_target_VERSION_PATCH@
#define UTOPIAOS_TARGET_VERSION_TWEAK @UtopiaOS_target_VERSION_TWEAK@

#define UTOPIAOS_TARGET_VERSION_HUMAN @UtopiaOS_target_VERSION@
#define UTOPIAOS_TARGET_VERSION_CXX @UtopiaOS_target_VERSION_MAJOR@ ## x ## \
                                    @UtopiaOS_target_VERSION_MINOR@ ## x ## \
                                    @UtopiaOS_target_VERSION_PATCH@ ## x ## \
                                    @UtopiaOS_target_VERSION_TWEAK@

#endif
