/** \defgroup utils Utilities
 * \brief This module contains general purpose
 *        templates, types and functions.
 * \{
 *
 * \file utils/assert.hpp
 * \brief This file defines a simple runtime
 *        assertion API.
 */

#ifndef H_utils_assert
#define H_utils_assert

#include <target/config.hpp>

#include <io/logger.hpp>

#include <utility>

#include "trap.hpp"

namespace UtopiaOS
{
    namespace utils
    {
        /** \brief Simple assertion function.
         * \tparam STRING Some \a logger compatible string type
         * \param[in] assertion The assertion
         * \param[in] error_message The error message to print
         *            if the assertion was false.
         */
        template<class STRING>
        void runtime_assert( bool assertion, STRING &&error_message )
        {
            if( assertion == false )
            {
#if UTOPIAOS_HOSTED
                io::log( nullptr, "Assertion failed: ",
                        std::forward<STRING>( error_message ) );
#else
#warning "No hosted environment. Failed assertions will not produce error messages."
#endif
                trap();
            }
        }
    }
}

#endif

/** \} */
