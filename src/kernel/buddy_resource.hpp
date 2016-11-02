#/** \ingroup kernel
* \{
*
* \file kernel/buddy_resource.hpp
* \brief This file declares the \a buddy_resource
*        class, that implements memory allocation
*        through the buddy method.
*/

#ifndef H_kernel_buddy_resource
#define H_kernel_buddy_resource

#include <utils/bitwise.hpp>

#include <memory_resource>
#include <limits>
#include <cstddef>
#include <utility>

namespace UtopiaOS
{
    namespace kernel
    {
        /** \class buddy_resource
         * \brief A conforming subclass of \a std::pmr::memory_resource that
         *        implements the 'buddy method'.
         * \note Every returned memory block will be aligned to
         *       \a alignof(std::max_align_t).
         * \tparam MinimumBlockSize Specifies the minimum block size handled
         *         by the \a buddy_resource. Has to be unzero, a power of two,
         *         less than or equal to \a MaximumBlockSize and large enough
         *         to hold the internal bookkepping information.
         * \tparam MaximumBlockSize Specifies the maximum block size handled
         *         by the \a buddy_resource. Has to be unzero, a power of two
         *         and greater than or equal to \a MinimumBlockSize.
         * \note If the above requirements are not fulfilled, a compile-time
         *       error will be issued.
         */
        template<std::size_t MinimumBlockSize, std::size_t MaximumBlockSize>
        class buddy_resource : public std::pmr::memory_resource
        {
            static_assert( (MinimumBlockSize & (MinimumBlockSize - 1)) == 0,
                          "The minimum block size has to be a power of two." );
            static_assert( (MaximumBlockSize & (MaximumBlockSize - 1)) == 0,
                          "The maximum block size has to be a power of two." );
            static_assert( MinimumBlockSize <= MaximumBlockSize,
                          "The minimum block size has to be less than or equal to the \
maximum block size." );
            static_assert( MinimumBlockSize != 0,
                          "The minimum block size cannot be zero." );
        private:
            static constexpr std::size_t min_block_size = MinimumBlockSize;
            static constexpr std::size_t max_block_size = MaximumBlockSize;
            
            static constexpr std::size_t max_msb = utils::msb( max_block_size );
            static constexpr std::size_t min_msb = utils::msb( min_block_size );
            static constexpr std::size_t max_block_level = max_msb - min_msb;
            static constexpr std::size_t num_block_levels = max_block_level + 1;
            
            std::pmr::memory_resource *upstream;
            
            struct memory_block_info
            {
                memory_block_info *buddy;
                bool is_free;
                
                memory_block_info *next;
            };
            
            static constexpr std::size_t max_align = alignof(std::max_align_t);
            static constexpr std::size_t padding = ((max_align -
                                                     (sizeof(memory_block_info) % max_align))
                                                    % max_align);
            
            static_assert( min_block_size > sizeof(memory_block_info) + padding,
                          "Minimum block size must be larger than bookkeeping information." );
            
            std::array<memory_block_info *, num_block_levels> free_block_lists;
            
            virtual void* do_allocate( std::size_t bytes, std::size_t alignment )
            {
                /** \todo What happens when alignment is too large?
                 *        See http://stackoverflow.com/questions/40386696/behaviour-of-stdpmrmemory-resourceallocate-on-large-alignments
                 */
                
                if( bytes == 0 )
                    return nullptr;
                if( alignment > max_align )
                    throw std::bad_alloc();
                
                std::size_t required_size = bytes + padding + sizeof(memory_block_info);
                if( required_size > max_block_size )
                    throw std::bad_alloc();
                
                std::size_t block_msb = utils::msb( required_size );
                if( (1U << (block_msb - 1)) < required_size )
                    block_msb++;
                
                std::uintptr_t info_address = target::ptr_to_uintptr(
                                            allocate_block( block_msb - min_msb ) );
                return target::uintptr_to_ptr<void>( info_address + padding );
            }
            
            memory_block_info *allocate_block( std::size_t block_level )
            {
                memory_block_info *current = free_block_lists[block_level];
                if( current != nullptr )
                {
                    free_block_lists[block_level] = current->next;
                    return current;
                }
                
                if( block_level != max_block_level )
                {
                    auto split = split_block( allocate_block( block_level + 1 ),
                                             block_level + 1 );
                    free_block_lists[block_level] = split.first;
                    return split.second;
                }
                
                void *memory = upstream->allocate( 1U << (max_block_level + min_msb - 1),
                                                  max_align );
                return reinterpret_cast<memory_block_info *>( memory );
            }
            
            std::pair<memory_block_info *, memory_block_info *>
            split_block( memory_block_info *block, std::size_t block_level )
            {
                std::size_t block_size = (1U << (block_level + min_msb - 1));
                std::uintptr_t info_address = target::ptr_to_uintptr( block );
                
                memory_block_info *first = block;
                memory_block_info *second = reinterpret_cast<memory_block_info *>(
                            target::uintptr_to_ptr<void>( info_address + (block_size >> 1) ) );
                
                first->is_free = second->is_free = false;
                first->buddy = second;
                second->buddy = first;
                
                return std::make_pair( first, second );
            }
            
            virtual void do_deallocate( void* p, std::size_t bytes, std::size_t alignment )
            {
                if( bytes == 0 )
                    return;
            }
            
            virtual bool do_is_equal( const std::pmr::memory_resource& other ) const;
        public:
            buddy_resource( std::pmr::memory_resource *upstream_resource )
            : upstream( upstream_resource ),
            free_block_lists( utils::make_array<num_block_levels, memory_block_info *>::copy( nullptr ) )
            {}
        };
    }
}

#endif

/** \} */
