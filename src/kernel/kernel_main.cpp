/** \ingroup kernel
 * \{
 *
 * \file kernel/kernel_main.cpp
 * \brief This file contains the boot code of the kernel.
 */

#include "kernel_main.hpp"

#include <target/config.hpp>
#include <environment/environment.hpp>
#include <common/types.hpp>

#include "trap.hpp"
#include "memory_manager.hpp"
#include "dynarray.hpp"

using namespace UtopiaOS;

namespace
{
    using namespace kernel;
    
    /** \todo Make this more sensible! */
    /** \brief This is the default OS X stack size (more or less arbitrary) */
    static constexpr common::un min_kernel_stack_size = (1 << 23);
    
    /** \brief Create a simple memory manager from the memory data.
     * \param[in] env The environment provided by the bootloader
     */
    unsynchronized_memory_manager setup_memory_manager( const environment *env );
    
    /** \brief Become a scheduler and start the memory managing process.
     * \param[in] mm The current memory manager whose memory should be used
     */
    [[noreturn]] void morph_into_scheduler_outsource_memory( unsynchronized_memory_manager &&mm );
}

[[noreturn]] void kernel::kernel_main( const environment *env )
{
    assert( env->kernel_stack_region.size >= min_kernel_stack_size,
           "Kernel stack size too small" );
    
    auto memory_pool = setup_memory_manager( env );
    morph_into_scheduler_outsource_memory( std::move( memory_pool ) );
}

namespace
{
    using namespace kernel;
    
    unsynchronized_memory_manager setup_memory_manager( const environment *env )
    {
        using namespace UtopiaOS;
        using namespace kernel;
        
        using memory_descriptor_allocator = std::pmr::polymorphic_allocator<memory_descriptor>;
        using kernel_memory_map = memory_map<memory_descriptor_allocator>;
        
        auto kernel_image_region = env->kernel_image_region;
        auto kernel_stack_region = env->kernel_stack_region;
        auto &UEFI_memmap = env->memmap;
        
        // First we alloca() enough space to convert the memory map
        // into our own format.
        auto memmap_memory_requirement = kernel_memory_map::maximum_conversion_requirement( UEFI_memmap );
        void *memmap_memory = UTOPIAOS_ALLOCA_WITH_ALIGN( memmap_memory_requirement.size,
                                                         memmap_memory_requirement.alignment );
        std::pmr::monotonic_buffer_resource memmap_memory_resource( memmap_memory,
                                                                   memmap_memory_requirement.size );
        
        // Next we create a preliminary memory map in the correct
        // format, but with a helplessly primitive allocator
        kernel_memory_map stage1_memmap( UEFI_memmap, &memmap_memory_resource );
        
        // Now before creating the memory manager, we need to
        // make sure it knows about already occupied memory.
        // We introduce the "occupied memory description": omd
        std::size_t omd_memory_size = 2 * sizeof(common::memory_region);
        void *omd_memory = UTOPIAOS_ALLOCA_WITH_ALIGN( omd_memory_size,
                                                      alignof(common::memory_region) );
        std::pmr::monotonic_buffer_resource omd_memory_resource( omd_memory,
                                                                omd_memory_size );
        
        using omd_allocator = std::pmr::polymorphic_allocator<common::memory_region>;
        dynarray<common::memory_region, omd_allocator> omd( {kernel_image_region,
            kernel_stack_region}, &omd_memory_resource );
        
        // Finally we can create the memory_manager!
        return unsynchronized_memory_manager( std::move( stage1_memmap ),
                                             omd.begin(), omd.end() );
    }
}

/** \} */
