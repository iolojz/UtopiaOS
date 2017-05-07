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

#include "target/memory.hpp"
#include "UEFI/memory.hpp"
#include "utils/make_array.hpp"

#include <boost/range/join.hpp>

namespace UtopiaOS
{
    /** \struct environment
     * \brief A struct containing all information that
     *        the kernel gets from the bootloader.
     * \note This structure needs to be API/ABI-stable
     *       and hence may not be changed!
     */
    struct environment
    {
        void *data; /**< A pointer to the data */
        UEFI::uint32 version; /**< The version of the structure */
        UEFI::uint32 least_compatible_version; /**< The minimum version that is still compatible */
    };
    
    /** \struct environment_v1
     * \brief A struct containing the information that
     *        the kernel gets from a version 1 compliant
     *        bootloader.
     * \note This structure needs to be API/ABI-stable
     *       and hence may not be changed!
     */
    struct environment_v1
    {
        /** \brief The memory region where the kernel
         *         binary is loaded */
        UEFI::memory_region kernel_image_region;
        /** \brief The memory region where the kernel
         *         stack is located */
        UEFI::memory_region kernel_stack_region;
        
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
            auto this_omd = std::array<target::memory_region, 1>{ target::memory_region{
                target::ptr_to_uintptr( this ),
                sizeof( environment )
            } };
            
            auto combined_view = boost::join( memmap_omd, this_omd );
            constexpr auto size = memmap_omd.size() + this_omd.size();
            
            return utils::make_array<size>::iterate( boost::begin( combined_view ) );
        }
    };
}

#endif

/** \} */
