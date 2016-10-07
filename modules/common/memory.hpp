/** \ingroup common
 * \{
 *
 * \file common/memory.hpp
 * \brief This file contains basic memory-related
 *        complete types
 */

#ifndef H_common_memory
#define H_common_memory

#include "types.hpp"

namespace UtopiaOS
{
    namespace common
    {
        /** \struct memory_region
         * \brief This struct represents a memory region
         *        in some address map, not necessarily the
         *        current one
         */
        struct memory_region
        {
            /** \brief A pointer to the start of the memory region */
            void *base;
            /** \brief The size of the memory region */
            un size;
        };
    }
}

#endif

/** \} */
