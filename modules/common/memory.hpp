#ifndef H_common_memory
#define H_common_memory

#include "types.hpp"

namespace JayZOS
{
    namespace common
    {
        struct memory_region
        {
            void *base;
            un size;
        };
    }
}

#endif
