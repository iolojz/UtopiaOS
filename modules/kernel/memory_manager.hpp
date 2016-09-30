#ifndef H_kernel_memory
#define H_kernel_memory

#include "memory_map.hpp"
#include "debug.hpp"

#include <algorithm>
#include <memory_resource>
#include <functional>

namespace UtopiaOS
{
    namespace kernel
    {
        // This class is intended to be an abstract base class
        // ensuring a meaningful interface
        class memory_manager
        {
        public:
            virtual ~memory_manager( void ) = default;
        };
        
        class unsynchronized_memory_manager : public memory_manager,
            public std::pmr::unsynchronized_pool_resource
        {
        public:
            // OMD stands for "occupied memory description"
            template<class InputIterator>
            unsynchronized_memory_manager( memory_map &&old_memmap,
                                          InputIterator OMDBegin, InputIterator OMDEnd )
            {
                // ASLR! Otherwise the memory maps location in physical
                // memory is pracitcally known beforehand!
                
                
                // We need more protection! Otherwise we might owerwrite the
                // stack or the the old memory map or...
                
                const auto copy_requirement = old_memmap.copy_requirement();
                const auto &old_descriptors = old_memmap.descriptors;
                
                auto it = std::find( old_descriptors.cbegin(), old_descriptors.cend(),
                                    std::bind( &memory_descriptor::can_meet_request,
                                              std::placeholders::_1, copy_requirement ) );
            }
        };
    }
}

#endif
