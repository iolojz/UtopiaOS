#ifndef H_common_types
#define H_common_types

#include <target/headers.hpp>

/********* Necessary compiler headers *********/
#include <HPATH_fixed_width_integers>
#include <HPATH_uintptr>
/**********************************************/

namespace JayZOS
{
    namespace common
    {
        using sn = signed;
        using un = unsigned;
        
        using uint32 = uint32_t;
        using uint64 = uint64_t;
        
        using uintptr = uintptr_t;
    }
}

#endif
