#ifndef H_kernel_debug
#define H_kernel_debug

#include <target/config.hpp>

#include "logger.hpp"
#include "trap.hpp"

namespace UtopiaOS
{
    namespace kernel
    {
        template<class STRING>
        void debug_assert( bool assertion, STRING &&error_message )
        {
#if UTOPIAOS_ENABLE_DEBUG_ASSERTS
            if( assertion == false )
            {
                log( assertion_logger, "Assertion failed: ",
                    std::forward<STRING>( error_message ) );
                trap();
            }
#endif
        }
    }
}

#endif
