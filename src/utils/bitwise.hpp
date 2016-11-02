#/** \ingroup utils
* \{
*
* \file utils/bitwise.hpp
* \brief This file defines a few bit operations
*        for integer types.
*/

#ifndef H_utils_bitwise
#define H_utils_bitwise

#include <type_traits>

namespace UtopiaOS
{
    namespace utils
    {
        template<class UInteger>
        constexpr std::size_t msb( UInteger val )
        {
            /** Check assembly and possibly optimize. */
            
            static_assert( std::is_unsigned<UInteger>::value,
                          "UInteger has to be an unsigned integer type." );
            
            std::size_t count = 0;
            while( val != 0 )
            {
                val >>= 1;
                count++;
            }
            
            return count;
        }
    }
}

#endif

/** \} */
