#ifndef H_UtopiaOS_UEFI_config
#define H_UtopiaOS_UEFI_config

#include "../config.hpp"

#define UTOPIAOS_UEFI_VERSION_MAJOR @UtopiaOS_UEFI_VERSION_MAJOR@
#define UTOPIAOS_UEFI_VERSION_MINOR @UtopiaOS_UEFI_VERSION_MINOR@
#define UTOPIAOS_UEFI_VERSION_PATCH @UtopiaOS_UEFI_VERSION_PATCH@
#define UTOPIAOS_UEFI_VERSION_TWEAK @UtopiaOS_UEFI_VERSION_TWEAK@

#define UTOPIAOS_UEFI_VERSION_HUMAN @UtopiaOS_UEFI_VERSION@
#define UTOPIAOS_UEFI_VERSION_CXX @UtopiaOS_UEFI_VERSION_MAJOR@ ## x ## \
                                  @UtopiaOS_UEFI_VERSION_MINOR@ ## x ## \
                                  @UtopiaOS_UEFI_VERSION_PATCH@ ## x ## \
                                  @UtopiaOS_UEFI_VERSION_TWEAK@

#endif
