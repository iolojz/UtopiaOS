/** \ingroup kernel
 * \{
 *
 * \file kernel/assert.hpp
 * \brief This file defines a simple runtime
 *        assertion API.
 */

#ifndef H_kernel_assert
#define H_kernel_assert

#include "logger.hpp"
#include "trap.hpp"

namespace UtopiaOS
{
    namespace kernel
    {
        /** \brief Simple assertion function.
         * \tparam STRING Some \a logger compatible string type
         * \param[in] assertion The assertion
         * \param[in] error_message The error message to print
         *            if the assertion was false.
         */
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

/** \} */
