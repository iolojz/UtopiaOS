/** \ingroup kernel
 * \{
 *
 * \file kernel/constants.hpp
 * \brief This file contains constant kernel
 *        configuration parameters.
 * \todo This should probably be linked
 *       to target/config.hpp
 */

#ifndef H_kernel_constants
#define H_kernel_constants

#include "target/target.hpp"
#include "utils/bitwise.hpp"

#include "buddy_resource.hpp"

namespace UtopiaOS
{
    namespace kernel
    {
        /* \brief The pagesize used by the kernel, which
         *        has to be unzero and a power of two!
         */
        static constexpr std::size_t pagesize = UTOPIAOS_KERNEL_PAGESIZE;
        
        namespace detail
        {
            static constexpr std::size_t pagesize_msb = utils::msb( pagesize );
            static constexpr std::size_t min_mem_chunk_msb =
                utils::msb( buddy_resource::min_allowed_block_size );
            
            static_assert( pagesize_msb >= min_mem_chunk_msb,
                "The pagesize is too small to support meaningful allocations." );
            
            /** \brief Magic number giving the maximum number
             *         of sub-page memory chunk levels.
             * \todo This should not be too magical.
             */
            static constexpr std::size_t max_mem_chunk_levels = 10;
            static constexpr std::size_t mem_chunk_levels = 
                std::min( pagesize_msb - min_mem_chunk_msb, max_mem_chunk_levels );
        }
        
        /* \brief The smallest memory chunk size that
         *        can be allocated by the predefined
         *        allocating facilities. It has to be
         *        unzero and a power of two!
         */
        static constexpr std::size_t smallest_memory_chunk =
            (pagesize >> detail::mem_chunk_levels);
        
        static_assert( pagesize != 0, "pagesize must not be zero" );
        static_assert( ((pagesize - 1) & pagesize) == 0,
                      "pagesize must be a power of two" );
    }
}

#endif

/** \} */
