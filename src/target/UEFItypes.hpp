/** \ingroup target
 * \{
 *
 * \file target/UEFItypes.hpp
 * \brief This file contains definitions of
 *        UEFI compatible types.
 */

#ifndef H_target_UEFItypes
#define H_target_UEFItypes

#include "config.hpp"

namespace UtopiaOS
{
    namespace target
    {
        namespace UEFI
        {
            /** \typedef un
             * \brief UEFI native unsigned integer type
             */
            using un = UTOPIAOS_UEFI_UN;
            
            /** \typedef uint32
             * \brief UEFI unsigned integer type
             *        with exactly 32 usable bits.
             */
            using uint32 = UTOPIAOS_UEFI_UINT32;
            
            /** \typedef uint64
             * \brief Signed integer type
             *        with exactly 32 usable bits.
             */
            using uint64 = UTOPIAOS_UEFI_UINT64;
        }
    }
}

#endif

/** \} */
