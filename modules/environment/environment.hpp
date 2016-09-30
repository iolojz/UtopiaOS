#ifndef H_environment_environment
#define H_environment_environment

#include <common/memory.hpp>
#include "memory.hpp"

namespace UtopiaOS
{
    struct environment
    {
        common::memory_region kernel_image_region;
        common::memory_region kernel_stack_region;
        
        UEFI::memory_map memmap;
    };
}

#endif
