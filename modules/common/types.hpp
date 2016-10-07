/** \defgroup common Common
 * \brief This module is header-only and contains basic
 *        types used throughout the OS.
 * \{
 *
 * \file common/types.hpp
 * \brief This file provides the most basic
 *        complete types that are used throughout the OS.
 */

#ifndef H_common_types
#define H_common_types

#include <target/headers.hpp>

/********* Necessary compiler headers *********/
#include HPATH_fixed_width_integers
#include HPATH_uintptr
/**********************************************/

/** \namespace UtopiaOS
 * \brief Everything OS-related is contained in this namespace.
 */
namespace UtopiaOS
{
    /** \namespace UtopiaOS::common
     * \brief This namespace contains basic stuff that is
     *        used across the OS.
     */
    namespace common
    {
        /** \typedef sn
         * \brief Native signed integer type
         */
        using sn = signed;
        
        /** \typedef un
         * \brief Native unsigned integer type
         */
        using un = unsigned;
        
        /** \typedef uint32
         * \brief Unsigned integer type
         *        with exactly 32 usable bits.
         */
        using uint32 = uint32_t;
        
        /** \typedef uint64
         * \brief Signed integer type
         *        with exactly 32 usable bits.
         */
        using uint64 = uint64_t;
        
        /** \typedef uintptr
         * \brief Unsigned integer type that can be
         * converted to and from all pointer types
         * using the appropriate conversion functions.
         */
        using uintptr = uintptr_t;
        
        /** \brief pointer to uintptr conversion function.
         * \tparam T The type of object the pointer points to (usually inferred).
         * \param[in] ptr The pointer that is to be converted
         * \return A uintptr carrying the value of \a ptr
         */
        template<class T>
        static inline uintptr ptr_to_uintptr( T *ptr )
        { return reinterpret_cast<uintptr>( ptr ); }
        
        /** \brief uintptr to pointer conversion function.
         * \tparam T The type of object the pointer should point to.
         * \param[in] u The uintptr that is to be converted
         * \return A T * pointing to the location specified by \a u
         */
        template<class T>
        static inline T *uintptr_to_ptr( uintptr u )
        { return reinterpret_cast<T*>( u ); }
        
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
