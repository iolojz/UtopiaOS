#/** \ingroup kernel
* \{
*
* \file kernel/trap.hpp
* \brief This file defines a function that causes
*        the thread of execution to stop immediately.
*/

#ifndef H_kernel_trap
#define H_kernel_trap

#include <target/config.hpp>

namespace UtopiaOS
{
    namespace kernel
    {
        /** \brief Causes an immediate halt of the current
         *        thread of execution.
         */
        [[noreturn]] static inline void trap( void )
        {
            UTOPIAOS_TRAP();
        }
    }
}

#endif

/** \} */
