#ifndef H_kernel_constants
#define H_kernel_constants

#include <target/config.hpp>

#include "types.hpp"

namespace UtopiaOS
{
    namespace kernel
    {
        static constexpr un pagesize = UTOPIAOS_KERNEL_PAGESIZE;
        
        static_assert( ((pagesize - 1) & pagesize) == 0,
                      "pagesize must be a power of two" );
    }
}

#endif
