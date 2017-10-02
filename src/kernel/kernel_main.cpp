/** \ingroup kernel
 * \{
 *
 * \file kernel/kernel_main.cpp
 * \brief This file contains the boot code of the kernel.
 */

#include "kernel_main.hpp"

#include "target/target.hpp"
#include "environment/environment.hpp"
#include "utils/trap.hpp"
#include "utils/assert.hpp"

#ifdef UTOPIAOS_ALLOCA_WITH_ALIGN_HEADER
#include UTOPIAOS_ALLOCA_WITH_ALIGN_HEADER
#endif

#include <new>
#include <algorithm>
#include <boost/range/join.hpp>

#include "memory_manager.hpp"

using namespace UtopiaOS;

namespace
{
    using namespace kernel;
    
    /** \todo Make this more sensible! */
    /** \brief This is the default OS X stack size (more or less arbitrary) */
    static constexpr unsigned min_kernel_stack_size = (1 << 23);
    
    /** \brief Create a simple memory manager from the memory data.
     * \param[in] env The environment provided by the bootloader
     */
    unsynchronized_memory_manager setup_memory_manager( const environment_v1 *env );
    
    /** \brief Become a scheduler and start the memory managing process.
     * \param[in] mm The current memory manager whose memory should be used
     */
    [[noreturn]] void morph_into_scheduler_outsource_memory( unsynchronized_memory_manager &&mm );
}

[[noreturn]] void kernel::kernel_main( const environment *env )
{
    utils::runtime_assert( env->least_compatible_version == 1,
                          "Environment has incompatible version." );
    
    auto environment = reinterpret_cast<const environment_v1 *>( env->data );
    utils::runtime_assert( environment->kernel_stack_region.size >= min_kernel_stack_size,
           "Kernel stack size too small" );
    
    /** \todo initialize certain essential parts of the c++ runtime:
     * - exception handling
     * - floating point ?
     * - memory stuff (new, delete, default_resource, ...)
     */
    
    auto memory_manager = setup_memory_manager( environment );
    morph_into_scheduler_outsource_memory( std::move( memory_manager ) );
}

namespace
{
    using namespace kernel;
    
    unsynchronized_memory_manager setup_memory_manager( const environment_v1 *env )
    {
        using namespace UtopiaOS;
        using namespace kernel;
        
        using memory_descriptor_allocator = std::pmr::polymorphic_allocator<memory_descriptor>;
        using kernel_memory_map = memory_map<memory_descriptor_allocator>;
        
        auto kernel_image_region = env->kernel_image_region;
        auto kernel_stack_region = env->kernel_stack_region;
        auto &UEFI_memmap = env->memmap;
        
        /** \todo Perform some runtime size check */
        
        auto memmap_memory_requirement = kernel_memory_map::maximum_conversion_requirement( UEFI_memmap );
        void *memmap_memory = UTOPIAOS_ALLOCA_WITH_ALIGN( memmap_memory_requirement.size,
                                                         memmap_memory_requirement.alignment );
        std::pmr::monotonic_buffer_resource memmap_memory_resource( memmap_memory,
                                                                   memmap_memory_requirement.size );
        
        kernel_memory_map memmap( UEFI_memmap, &memmap_memory_resource );
        
        auto environment_omd = env->occupied_memory();
        std::array<target::memory_region, 2> kernel_omd = { {
            target::memory_region( kernel_image_region ),
            target::memory_region( kernel_stack_region )
        } };
        
        auto omd_view = boost::join( environment_omd, kernel_omd );
        std::sort( boost::begin( omd_view ), boost::end( omd_view ) );
        
        return unsynchronized_memory_manager( memmap,
                                             boost::begin( omd_view ),
                                             boost::end( omd_view ) );
    }
}

/** \} */
