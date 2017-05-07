#ifndef H_UtopiaOS_kernel_config
#define H_UtopiaOS_kernel_config

#include "../config.hpp"

#define UTOPIAOS_KERNEL_VERSION_MAJOR @UtopiaOS_kernel_VERSION_MAJOR@
#define UTOPIAOS_KERNEL_VERSION_MINOR @UtopiaOS_kernel_VERSION_MINOR@
#define UTOPIAOS_KERNEL_VERSION_PATCH @UtopiaOS_kernel_VERSION_PATCH@
#define UTOPIAOS_KERNEL_VERSION_TWEAK @UtopiaOS_kernel_VERSION_TWEAK@

#define UTOPIAOS_KERNEL_VERSION_HUMAN @UtopiaOS_kernel_VERSION@
#define UTOPIAOS_KERNEL_VERSION_CXX @UtopiaOS_kernel_VERSION_MAJOR@ ## x ## \
                                    @UtopiaOS_kernel_VERSION_MINOR@ ## x ## \
                                    @UtopiaOS_kernel_VERSION_PATCH@ ## x ## \
                                    @UtopiaOS_kernel_VERSION_TWEAK@

#endif
