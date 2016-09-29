#ifndef H_kernel_debug
#define H_kernel_debug

#include <target/config.hpp>

#include "template_utilities.hpp"
#include "log.hpp"
#include "trap.hpp"

namespace JayZOS
{
    namespace kernel
    {
        template<class STRING>
        void debug_assert( bool assertion, STRING &&error_message )
        {
#if JAYZOS_ENABLE_DEBUG_ASSERTS
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
