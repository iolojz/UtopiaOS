/** \ingroup utils
 * \{
 *
 * \file utils/debug.hpp
 * \brief This file contains simple functions
 *        that can be used for debugging purposes.
 */

#ifndef H_utils_debug
#define H_utils_debug

#include <target/config.hpp>

#include "assert.hpp"

namespace UtopiaOS
{
    namespace utils
    {
        /** \brief Simple assertion function that only checks
         *         the assertion when the compile-time debug flag
         *         UTOPIAOS_ENABLE_DEBUG_ASSERTS is set.
         * \tparam STRING Some \a logger compatible string type
         * \param[in] assertion The assertion
         * \param[in] error_message The error message to print
         *            if the assertion was false.
         */
        template<class STRING>
        void debug_assert( bool assertion, STRING &&error_message )
        {
#if UTOPIAOS_ENABLE_DEBUG_ASSERTS
            assert( assertion, std::forward<STRING>( error_message ) );
#endif
        }
    }
}

#endif

/** \} */
