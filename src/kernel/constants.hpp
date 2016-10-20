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

#include <target/config.hpp>

namespace UtopiaOS
{
    namespace kernel
    {
        /* \brief The pagesize used by the kernel, which
         *        has to be unzero and a power of two!
         */
        static constexpr unsigned pagesize = UTOPIAOS_KERNEL_PAGESIZE;
        
        static_assert( pagesize != 0, "pagesize must not be zero" );
        static_assert( ((pagesize - 1) & pagesize) == 0,
                      "pagesize must be a power of two" );
    }
}

#endif

/** \} */
