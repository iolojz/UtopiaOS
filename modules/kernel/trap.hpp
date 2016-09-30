#ifndef H_kernel_trap
#define H_kernel_trap

#include <target/config.hpp>

namespace UtopiaOS
{
    namespace kernel
    {
        [[noreturn]] static inline void trap( void )
        {
            UTOPIAOS_TRAP();
        }
    }
}

#endif
