/** \defgroup io IO
 * \brief This module contains io-related constructions.
 * \{
 *
 * \file io/string.hpp
 * \brief This file contains the definitions
 *        of text-related types used throughout
 *        the OS.
 */

#ifndef H_io_string
#define H_io_string

namespace UtopiaOS
{
    /** \namespace UtopiaOS::io
     * \brief Everything in the module IO module
     *        goes into this namespace.
     */
    namespace io
    {
        /** \typedef stringref
         * \brief A type that behaves like a reference to a string.
         * \warning This type knows no ownershop semantics.
         */
        using stringref = char *;
        
        /** \typedef const_stringref
         * \brief A type that behaves like a reference to a const string.
         * \warning This type knows no ownershop semantics.
         */
        using const_stringref = const char *;
    }
}

#endif

/** \} */
