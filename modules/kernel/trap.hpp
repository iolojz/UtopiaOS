#ifndef H_kernel_trap
#define H_kernel_trap

#include <target/config.hpp>

namespace JayZOS
{
    namespace kernel
    {
        static inline void trap( void ) [[noreturn]]
        {
            JAYZOS_TRAP();
        }
    }
}

#endif
