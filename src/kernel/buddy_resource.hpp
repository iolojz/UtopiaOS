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

#include "utils/bitwise.hpp"
#include "target/memory.hpp"

#include <memory_resource>

namespace UtopiaOS
{
    namespace kernel
    {
        namespace detail
        {
            static constexpr std::size_t max_align = alignof(std::max_align_t);
            
            std::size_t block_size_at_level( std::size_t level, std::size_t min_msb )
            {
                return (std::size_t(1U) << (level + min_msb - 1));
            }
        
            struct memory_block_info
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
                
                bool is_occupied( void ) const
                { return !is_free(); }
                
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
                    std::size_t block_size = block_size_at_level( level, min_msb );
                    std::uintptr_t info_address = target::ptr_to_uintptr( this );
                    
                    if( is_first( level ) )
                        return target::uintptr_to_ptr<memory_block_info>(
                                                    info_address + (block_size >> 1) );
                    
                    return target::uintptr_to_ptr<memory_block_info>(
                                                 info_address - (block_size >> 1) );
                }
                
                void *data() const
                {
                    std::uintptr_t info_address = target::ptr_to_uintptr( this );
                    return target::uintptr_to_ptr<void>(
                        target::align<max_align>( info_address + sizeof(memory_block_info) ) );
                }
            };
            
            static constexpr std::size_t inverse_padding =
                (sizeof(memory_block_info) % max_align);
            static constexpr std::size_t padding =
                ((max_align - inverse_padding) % max_align);
        }
    
        /** \class buddy_resource
         * \brief A conforming subclass of \a std::pmr::memory_resource that
         *        implements the 'buddy method'.
         */
        class buddy_resource : public std::pmr::memory_resource
        {
        public:
            static constexpr std::size_t min_allowed_block_size =
                2 * (sizeof(detail::memory_block_info) + detail::padding);
            static constexpr std::size_t max_num_allowed_block_levels =
                utils::msb( std::numeric_limits<std::size_t>::max() ) - 1;
        private:
            std::size_t min_block_size;
            std::size_t max_block_size;
            
            std::size_t max_msb = utils::msb( max_block_size );
            std::size_t min_msb = utils::msb( min_block_size );
            std::size_t max_block_level = max_msb - min_msb;
            std::size_t num_block_levels = max_block_level + 1;
            
            std::size_t top_level_block_alignment;
            
            std::pmr::memory_resource *upstream;
            
            std::size_t block_lists_size;
            detail::memory_block_info **free_block_lists;
            
            /** \brief As specified by the c++ standard
             * \warning Alignment will always be to \a alignof(std::max_align_t).
             */
            virtual void* do_allocate( std::size_t bytes, std::size_t alignment );
            
            /** \brief Returns the block level necessary to satisfy
             *         a given allocation request.
             * \param[in] bytes The number of bytes requested
             * \param[in] alignment The requested alignment
             * \returns The block level necessary to satisfy
             *         a given allocation request.
             * \warning Alignment will always be to \a alignof(std::max_align_t).
             */
            std::size_t level_for_allocation_request( std::size_t bytes,
                                            std::size_t alignment ) const;
            
            /** \brief Allocates a block of the specified level
             * \param[in] block_level The level of the block to be allocated.
             * \returns A memory block of the given level.
             *
             * \note The returned memory block will always be occupied.
             */
            detail::memory_block_info *allocate_block( std::size_t block_level );
            
            /** \brief Splits a given memory block into two buddies.
             * \param[inout] block The given memory block
             * \param[in] block_level The level of the given block
             * \returns Two buddies corresponding to memory blocks
             *          of one level less than \a block_level that
             *          are both occupied.
             * \note \a block has to be occupied otherwise the
             *       behaviour is undefined.
             * \note \a block_level has to correspond to the actual
             *       level of the block and be larger than zero
             *       otherwise the behaviour is undefined.
             */
            std::pair<detail::memory_block_info *, detail::memory_block_info *>
            split_block( detail::memory_block_info *block, std::size_t block_level );
            
            /** \brief As specified in the c++ standard.
             * \note This function never returns memory to
             *       its upstreams resource.
             */
            virtual void do_deallocate( void* p, std::size_t bytes,
                                       std::size_t alignment );
            
            /** \brief Deallocates a block of the specified level
             * \param[inout] block The block to be deallocated.
             * \param[in] block_level The level of the block to be deallocated.
             *
             * \note \a block has to be occupied otherwise the
             *       behaviour is undefined.
             * \note \a block_level has to correspond to the actual
             *       level of the block otherwise the behaviour is
             *       undefined.
             */
            void deallocate_block( detail::memory_block_info *block,
                                  std::size_t block_level );
            
            /** \brief Combines two given buddies of a specified level
             *         into one block of a higher level.
             * \param[inout] first A memory block.
             * \param[inout] second The buddy of \a first.
             * \returns An occupied memory block corresponding to the
             *          union of both given blocks with one level higher.
             * \note If the blocks are not buddies the behaviour is
             *       undefined.
             * \note \a block_level has to correspond to the actual
             *       level of the blocks otherwise the behaviour is
             *       undefined.
             */
            detail::memory_block_info *combine_buddies(
                detail::memory_block_info *first,
                detail::memory_block_info *second,
                std::size_t block_level );
            
            virtual bool do_is_equal( const std::pmr::memory_resource& other ) const;
        public:
            /** \brief Constructs a \a buddy_resource object
             * \param[in] min_bs The minimum block size.
             * \param[in] max_bs The maximum block size.
             * \param[in] tlp_alignment The alignment of the
             *                          top-level blocks.
             * \param[in] upstream_resource The upstream resource.
             *
             * \throws std::invalid_argument if the parameters
             *         unsupported.
            */
            buddy_resource( std::size_t min_bs, std::size_t max_bs,
                           std::size_t tlp_alignment,
                           std::pmr::memory_resource *upstream_resource );
            
            /** \brief A std::pmr::memory_resource should not be
             *         copyable.
             */
            buddy_resource( const buddy_resource & ) = delete;
            
            /** \brief A std::pmr::memory_resource should not be
             *         movable.
             */
            buddy_resource( buddy_resource && ) = delete;
            
            virtual ~buddy_resource( void );
        };
    }
}

#endif

/** \} */
