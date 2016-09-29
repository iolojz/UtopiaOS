#ifndef H_kernel_assert
#define H_kernel_assert

#include "template_utilities.hpp"
#include "logger.hpp"
#include "trap.hpp"

namespace JayZOS
{
    namespace kernel
    {
        template<class STRING>
        void assert( bool assertion, STRING &&error_message )
        {
            if( assertion == false )
            {
                log( assertion_logger, "Assertion failed: ",
                    std::forward<STRING>( error_message ) );
                trap();
            }
        }
    }
}

#endif
