/** \defgroup environment Environment
 * \brief This module is header-only and models the
 *        information that the kernel receives upon
 *        boot time.
 * \{
 *
 * \file environment/environment.hpp
 * \brief This file contains the type definition of
 *        the argument that is passed to the kernel
 *        upon boot time.
 */

#ifndef H_environment_environment
#define H_environment_environment

#include <common/memory.hpp>
#include "UEFIMemory.hpp"

namespace UtopiaOS
{
    /** \struct environment
     * \brief A struct containing all information that
     *        the kernel gets from the bootloader.
     */
    struct environment
    {
        /** \brief The memory region where the kernel
         *         binary is loaded */
        common::memory_region kernel_image_region;
        /** \brief The memory region where the kernel
         *         stack is located */
        common::memory_region kernel_stack_region;
        
        /** \brief The UEFI memory map */
        UEFI::memory_map memmap;
    };
}

#endif

/** \} */
