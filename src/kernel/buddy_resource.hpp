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

#include <target/target.hpp>

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

namespace UtopiaOS
{
    namespace kernel
    {
        /** \class buddy_resource
         * \brief A conforming subclass of \a std::pmr::memory_resource that
         *        implements the 'buddy method'.
         */
        class buddy_resource : public std::pmr::memory_resource
        {
        private:
            std::size_t min_block_size;
            std::size_t max_block_size;
            
            std::size_t max_msb = utils::msb( max_block_size );
            std::size_t min_msb = utils::msb( min_block_size );
            std::size_t max_block_level = max_msb - min_msb;
            std::size_t num_block_levels = max_block_level + 1;
            
            std::pmr::memory_resource *upstream;
            
            struct memory_block_info;
            memory_block_info **free_block_lists;
            
            virtual void* do_allocate( std::size_t bytes, std::size_t alignment );
            
            /** \brief Allocates a block of the specified level
             * \param[in] block_level The level of the block to be allocated.
             * \returns A memory block of the given level.
             *
             * \note The returned memory block will always be occupied.
             */
            memory_block_info *allocate_block( std::size_t block_level );
            
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
            std::pair<memory_block_info *, memory_block_info *>
            split_block( memory_block_info *block, std::size_t block_level );
            
            virtual void do_deallocate( void* p, std::size_t bytes, std::size_t alignment );
            
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
            void deallocate_block( memory_block_info *block, std::size_t block_level );
            
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
            memory_block_info *combine_buddies( memory_block_info *first,
                                               memory_block_info *second,
                                               std::size_t block_level );
            
            virtual bool do_is_equal( const std::pmr::memory_resource& other ) const;
        public:
            /** \brief Constructs a \a buddy_resource object
             * \param[in] min_bs The minimum block size.
             * \param[in] max_bs The maximum block size.
             * \param[in] upstream_resource The upstream resource.
             *
             * \note \a min_block_size has to be smaller than
             *       or equal to \a max_block_size.
             * \note \a min_block_size must be larger than the
             *       per-block bookkeeping information.
             * \note \a min_block_size and \a max_block_size
             *       must be powers of two.
             * \throws std::invalid_argument if any of the above
             *         is not fulfilled.
            */
            buddy_resource( std::size_t min_bs, std::size_t max_bs,
                           std::pmr::memory_resource *upstream_resource );
        };
    }
}

#endif

/** \} */
