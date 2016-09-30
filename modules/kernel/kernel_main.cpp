#include "kernel_main.hpp"

#include <target/config.hpp>
#include <environment/environment.hpp>

#include "types.hpp"
#include "trap.hpp"
#include "memory_manager.hpp"
#include "dynarray.hpp"

using namespace UtopiaOS;

namespace
{
    // This is the default OS X stack size (more or less arbitrary)
    static constexpr un min_kernel_stack_size = (1 << 23);
    
    /** setup_global_memory_allocation:
     *
     * From the memory data in env, create a global sequential
     * memory manager.
     **/
    unsynchronized_memory_manager setup_memory_manager( const environment *env );
}

void kernel::kernel_main( const environment *env ) [[noreturn]]
{
    // Make sure the stack is large enough!
    assert( env->kernel_stack_region >= min_kernel_stack_size,
           "Kernel stack size too small" );
    
    auto unsynchronized_memmanager = setup_memory_manager( env );
    
    // Now that we have set up unsynchronized memory management,
    // we need to do the obvious, go synchronized:
    // morph this process into a scheduler and launch a
    // separate memory managment process!
    // But before doing that, we need to create a list of
    // things for the scheduler to schedule. Otherwise it
    // will just loop doing absolutely nothing useful at all.
    
    morph_into_scheduler_outsource_memory( std::move( stage1_memmanager ) );
    
    // We will never arrive at this point. The scheduler
    // will always have work to do (scheduling...).
}

unsynchronized_memory_manager setup_memory_manager( const environment *env )
{
    using namespace UtopiaOS;
    using namespace kernel;
    
    auto kernel_image_region = env->kernel_image_region;
    auto kernel_stack_region = env->kernel_stack_region;
    auto &UEFI_memmap = env->memmap;
    
    // First we alloca() enough space to convert the memora map
    // into our own format.
    auto memmap_memory_requirement = memory_map::maximum_conversion_requirement( UEFI_memmap );
    void *memmap_memory = UTOPIAOS_ALLOCA_WITH_ALIGN( memmap_memory_requirement.size,
                                                   memmap_memory_requirement.alignment );
    std::pmr::monotonic_buffer_resource memmap_memory_resource( memmap_memory,
                                                               memmap_memory_requirement.size );
    
    // Next we create a preliminary memory map in the correct
    // format, but with a helplessly primitive allocator
    using memory_descriptor_allocator = std::pmr::polymorphic_allocator<memory_descriptor>;
    memory_map<memory_descriptor_allocator> stage1_memmap( UEFI_memmap, &memmap_memory_resource );
    
    // Now before creating the memory manager, we need to
    // make sure it knows about already occupied memory.
    // We introduce the "occupied memory description": OMD
    void *OMD_memory = UTOPIAOS_ALLOCA_WITH_ALIGN( 2 * sizeof(common::memory_region),
                                                  alignof(common::memory_region) );
    std::pmr::monotonic_buffer_resource OMD_memory_resource( memmap_memory,
                                                            memmap_memory_requirement.size );
    
    using OMD_allocator = std::pmr::polymorphic_allocator<common::memory_region>;
    dynarray<common::memory_region> OMD( {kernel_image_region, kernel_stack_region},
                                        &OMD_memory_resource );
    
    // Finally we can create the memory_manager! As we have not yet
    // set up scheduling/threading, we have to go for the
    // unsychronized version.
    return unsynchronized_memory_manager( std::move( stage1_memmap ), OMD );
}

