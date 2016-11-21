#/** \ingroup kernel
* \{
*
* \file kernel/buddy_resource.cpp
* \brief This file implements the \a buddy_resource
*        class, that implements memory allocation
*        through the buddy method.
*/

#include "buddy_resource.hpp"

#include <target/config.hpp>
#include <target/memory.hpp>

#ifdef UTOPIAOS_ALLOCA_WITH_ALIGN_HEADER
#include UTOPIAOS_ALLOCA_WITH_ALIGN_HEADER
#endif

#include <utils/bitwise.hpp>
#include <utils/debug.hpp>

#include <memory_resource>
#include <limits>
#include <cstddef>
#include <utility>
#include <cstring>

using namespace UtopiaOS;
using namespace kernel;

struct buddy_resource::memory_block_info
{
    std::size_t block_flags;
    memory_block_info *previous, *next;
    
private:
    static constexpr std::size_t msb = utils::msb(
                        std::numeric_limits<std::size_t>::max() );
    
public:
    void set_free( void )
    { block_flags |= (std::size_t(1U) << (msb - 1)); }
    void set_occupied( void )
    { block_flags &= ~(std::size_t(1U) << (msb - 1)); }
    
    bool is_free( void ) const
    { return ((block_flags & (std::size_t(1U) << (msb - 1))) != 0); }
    
    void set_first( std::size_t level )
    { block_flags |= (std::size_t(1U) << level); }
    
    void set_second( std::size_t level )
    { block_flags &= ~(std::size_t(1U) << level); }
    
    bool is_first( std::size_t level ) const
    { return ((block_flags & (std::size_t(1U) << level)) != 0); }
    
    bool is_second( std::size_t level ) const
    { return !is_first( level ); }
    
    memory_block_info *buddy( std::size_t level, std::size_t min_msb ) const
    {
        std::size_t block_size = (std::size_t(1U) << (level + min_msb - 1));
        std::uintptr_t info_address = target::ptr_to_uintptr( this );
        
        if( is_first( level ) )
            return target::uintptr_to_ptr<memory_block_info>(
                                        info_address + (block_size >> 1) );
        
        return target::uintptr_to_ptr<memory_block_info>(
                                     info_address - (block_size >> 1) );
    }
};

buddy_resource::memory_block_info *
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
    
    void *memory = upstream->allocate( 1U << (max_block_level + min_msb - 1),
                                      alignof(memory_block_info) );
    memory_block_info *info = reinterpret_cast<memory_block_info *>( memory );
    info->set_occupied();
    
    return info;
}

std::pair<
    buddy_resource::memory_block_info *,
    buddy_resource::memory_block_info *
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
    
    std::size_t padding = alignment - (alignof(memory_block_info) % alignment);
    
    std::size_t required_size = bytes + padding + sizeof(memory_block_info);
    std::size_t block_msb = utils::msb( required_size );
    if( (1U << (block_msb - 1)) < required_size )
        block_msb++;
    
    std::uintptr_t info_address = (target::ptr_to_uintptr( p ) -
                                   (padding + sizeof(memory_block_info)));
    deallocate_block( target::uintptr_to_ptr<memory_block_info>( info_address ),
                     block_msb - min_msb );
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

buddy_resource::memory_block_info *
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
                               std::pmr::memory_resource *upstream_resource )
: min_block_size( min_bs ), max_block_size( max_bs ),
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
    
    if( min_block_size <= sizeof(memory_block_info) )
        throw std::invalid_argument( "The minimum block size has to be larger \
                                    than the per-block bookkeeping information." );
    
    // Num block levels will always be bounded by the number of bytes
    // in std::size_t.
    std::size_t block_lists_size = num_block_levels * sizeof(memory_block_info *);
    
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

/** \} */
