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

#include <target/memory.hpp>
#include <UEFI/memory.hpp>

#include <array>
#include <boost/range/join.hpp>

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
        target::memory_region kernel_image_region;
        /** \brief The memory region where the kernel
         *         stack is located */
        target::memory_region kernel_stack_region;
        
        /** \brief The UEFI memory map */
        UEFI::memory_map memmap;
        
        /** \brief Returns the memory regions occpied by the environment
         *         structure.
         * \returns The memory regions occupied by the environment
         *          object and all subobjects.
         */
        auto occupied_memory( void ) const
        {
            auto memmap_omd = memmap.occupied_memory();
            auto object_omd = std::array<target::memory_region, 1>{ target::memory_region{
                target::ptr_to_uintptr( this ),
                sizeof( environment )
            } };
            
            return boost::join( memmap_omd, object_omd );
        }
    };
}

#endif

/** \} */
