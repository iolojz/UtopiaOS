#/** \ingroup utils
* \{
*
* \file utils/trap.hpp
* \brief This file defines a function that causes
*        the thread of execution to stop immediately.
*/

#ifndef H_utils_trap
#define H_utils_trap

#include <target/target.hpp>

namespace UtopiaOS
{
    namespace utils
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
