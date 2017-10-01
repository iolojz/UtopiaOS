#/** \ingroup kernel
* \{
*
* \file kernel/buddy_resource.cpp
* \brief This file implements the \a buddy_resource
*        class, that implements memory allocation
*        through the buddy method.
*/

#include "buddy_resource.hpp"

#include "target/target.hpp"
#include "target/memory.hpp"

#ifdef UTOPIAOS_ALLOCA_WITH_ALIGN_HEADER
#include UTOPIAOS_ALLOCA_WITH_ALIGN_HEADER
#endif

#include "utils/debug.hpp"

#include <memory_resource>
#include <limits>
#include <algorithm>
#include <exception>

using namespace UtopiaOS;
using namespace kernel;

using detail::max_align;
using detail::memory_block_info;
using detail::block_size_at_level;
using detail::padding;

std::size_t buddy_resource::level_for_allocation_request( std::size_t bytes, std::size_t ) const
{
    auto required_size = bytes + padding + sizeof(memory_block_info);
    auto msb = utils::msb( required_size );
    std::size_t level = msb - min_msb;
    
    if( ((required_size - 1) & required_size) != 0 )
        level++;
    
    return level;
}

void *buddy_resource::do_allocate( std::size_t bytes, std::size_t alignment )
{
    if( bytes == 0 )
        return nullptr;
    
    auto level = level_for_allocation_request( bytes, alignment );
    
    if( level > max_block_level )
        throw std::bad_alloc();
    
    auto block_info = allocate_block( level );
    return block_info->data();
}

memory_block_info *
buddy_resource::allocate_block( std::size_t block_level )
{
    utils::debug_assert( block_level <= max_block_level,
                        "Block level is larger than maximum block level." );
    
    memory_block_info *current = free_block_lists[block_level];
    if( current != nullptr )
    {
        free_block_lists[block_level] = current->next;
        free_block_lists[block_level]->previous = nullptr;
        
        current->set_occupied();
        return current;
    }
    
    if( block_level != max_block_level )
    {
        auto buddies = split_block( allocate_block( block_level + 1 ),
                                   block_level + 1 );
        free_block_lists[block_level] = buddies.first;
        buddies.first->previous = buddies.first->next = nullptr;
        buddies.first->set_free();
        
        return buddies.second;
    }
    
    void *memory = upstream->allocate( detail::block_size_at_level( max_block_level, min_msb ),
                                      top_level_block_alignment );
    
    if( (target::ptr_to_uintptr( memory ) % top_level_block_alignment) != 0 )
    {
        upstream->deallocate( memory,
                             block_size_at_level( max_block_level, min_msb ),
                             top_level_block_alignment );
        throw std::bad_alloc();
    }
    
    memory_block_info *info = reinterpret_cast<memory_block_info *>( memory );
    info->set_occupied();
    
    return info;
}

std::pair<
    memory_block_info *,
    memory_block_info *
>
buddy_resource::split_block( memory_block_info *block, std::size_t block_level )
{
    utils::debug_assert( block_level != 0,
                        "Cannot split 0-level block." );
    utils::debug_assert( block_level <= max_block_level,
                        "Block level is larger than maximum block level." );
    
    std::size_t block_size = (1U << (block_level + min_msb - 1));
    std::uintptr_t info_address = target::ptr_to_uintptr( block );
    
    memory_block_info *first = block;
    memory_block_info *second = reinterpret_cast<memory_block_info *>(
                target::uintptr_to_ptr<void>( info_address + (block_size >> 1) ) );
    
    *second = *first;
    first->set_first( block_level - 1 );
    second->set_second( block_level - 1 );
    
    return std::make_pair( first, second );
}

void buddy_resource::do_deallocate( void* p, std::size_t bytes, std::size_t alignment )
{
    if( bytes == 0 )
        return;
    
    std::uintptr_t info_address = (target::ptr_to_uintptr( p ) -
                                   (padding + sizeof(memory_block_info)));
    deallocate_block( target::uintptr_to_ptr<memory_block_info>( info_address ),
                     level_for_allocation_request( bytes, alignment ) );
}

void buddy_resource::deallocate_block( memory_block_info *block, std::size_t block_level )
{
    utils::debug_assert( block_level <= max_block_level,
                        "Block level is larger than maximum block level." );
    
    do
    {
        memory_block_info *buddy = block->buddy( block_level, min_msb );
        
        if( block_level == max_block_level || buddy->is_free() == false )
        {
            block->next = free_block_lists[block_level];
            block->previous = nullptr;
            free_block_lists[block_level] = block;
            
            if( block->next != nullptr )
                block->next->previous = block;
            
            block->set_free();
            return;
        }
        
        if( buddy->previous == nullptr )
        {
            free_block_lists[block_level] = buddy->next;
            if( free_block_lists[block_level] != nullptr )
                free_block_lists[block_level]->previous = nullptr;
        } else
        {
            buddy->previous->next = buddy->next;
            if( buddy->next != nullptr )
                buddy->next->previous = buddy->previous;
        }
        
        buddy->set_occupied();
        block = combine_buddies( block, buddy, block_level++ );
    } while( true );
}

memory_block_info *
buddy_resource::combine_buddies( memory_block_info *first,
                                memory_block_info *second,
                                std::size_t block_level )
{
    utils::debug_assert( block_level < max_block_level,
                        "The block level must be smaller than \
                        the maximum block level." );
    
    if( first->is_second( block_level ) )
        return second;
    
    return first;
}

bool buddy_resource::do_is_equal( const std::pmr::memory_resource& other ) const
{
    return (std::addressof( other ) == this);
}

buddy_resource::buddy_resource( std::size_t min_bs,
                               std::size_t max_bs,
                               std::size_t tlp_alignment,
                               std::pmr::memory_resource *upstream_resource )
: min_block_size( min_bs ), max_block_size( max_bs ),
top_level_block_alignment( std::max( tlp_alignment, max_align ) ),
upstream( upstream_resource )
{
    if( min_block_size > max_block_size )
        throw std::invalid_argument( "The minimum block size has to be less than \
or equal to the maximum block size." );
    if( ((min_block_size - 1) & min_block_size) != 0 )
        throw std::invalid_argument( "The minimum block size has to be a \
power of two." );
    if( ((max_block_size - 1) & max_block_size) != 0 )
        throw std::invalid_argument( "The maximum block size has to be a \
power of two." );
    
    if( min_block_size <= sizeof(memory_block_info) + padding )
        throw std::invalid_argument( "The minimum block size has to be larger \
than the per-block bookkeeping information." );
    if( num_block_levels > max_num_allowed_block_levels )
        throw std::invalid_argument( "Too many block levels." );
    if( std::numeric_limits<std::size_t>::max() / num_block_levels < sizeof(memory_block_info *) )
        throw std::invalid_argument( "Too many block levels." );
    
    block_lists_size = num_block_levels * sizeof(memory_block_info *);
    
    /** \todo Some runtime size check */
    
    free_block_lists = reinterpret_cast<memory_block_info **>(
                        UTOPIAOS_ALLOCA_WITH_ALIGN( block_lists_size,
                                                   alignof(memory_block_info *) ) );
    for( std::size_t level = 0; level < num_block_levels; level++ )
        free_block_lists[level] = nullptr;
    
    void *block_lists_memory = allocate( block_lists_size,
                                        alignof(memory_block_info *) );
    
    std::memcpy( block_lists_memory, free_block_lists, block_lists_size );
    free_block_lists = reinterpret_cast<memory_block_info **>( block_lists_memory );
}

buddy_resource::~buddy_resource( void )
{
    /** \todo Some runtime size check */
    
    auto old_free_block_lists = reinterpret_cast<memory_block_info **>(
                        UTOPIAOS_ALLOCA_WITH_ALIGN( block_lists_size,
                                                   alignof(memory_block_info *) ) );
    std::memcpy( old_free_block_lists, free_block_lists, block_lists_size );
    std::swap( free_block_lists, old_free_block_lists );
    
    deallocate( free_block_lists, block_lists_size, alignof(memory_block_info *) );
    
    for( std::size_t level = 0; level != max_block_level; level++ )
    {
        for( auto current = free_block_lists[level]; current != nullptr;
            current = free_block_lists[level] )
            deallocate_block( current, level );
    }
    
    for( auto current = free_block_lists[max_block_level]; current != nullptr;
        current = current->next )
        upstream->deallocate( current, 1U << (max_block_level + min_msb - 1),
                             alignof(memory_block_info) );
}

/** \} */
