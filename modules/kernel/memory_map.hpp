#ifndef H_kernel_memory
#define H_kernel_memory

#include "types.hpp"
#include "dynarray"

#include <memory_resource>
#include <algorithm>

namespace UtopiaOS
{
    namespace kernel
    {
        enum class memory_type : uint32
        {
            general_purpose,
            unusable
        };
        
        struct memory_request
        {
            uint64 size;
            un alignment;
        };
        
        // This only works when alignment is a power of two!
        static inline uintptr align( uintptr ptr, un alignment )
        {
            debug_assert( (alignment % 2) == 0, "alignment has to be a power of two" );
            
            un mask = alignment - 1; // ...0000011111...
            un diff = (ptr & mask);
            
            if( diff == 0 )
                return ptr;
            
            return (ptr + alignment - diff);
        }
        
        struct memory_descriptor
        {
            memory_type type;
            uint64 physical_start;
            uint64 virtual_start;
            uint64 number_of_pages;
            
            memory_descriptor( const UEFI::memory_descriptor_v1 &uefi_desc );
            
            bool can_meet_request( const memory_request &request ) const
            {
                if( type != memory_type::general_purpose )
                    return false;
                
                aligned_address = align( virtual_start, request.alignment );
                uintptr request_end = aligned_address + request.size;
                
                if( request_end <= virtual_start + number_of_pages * pagesize )
                    return true;
                
                return false;
            }
        };
        
        /** The memory map format used by the kernel
         * It has some sanity guarantees that UEFI lacks, like:
         * - fixed size of descriptors that is known compile-time
         * - no overlaps
         **/
        class memory_map final
        {
            friend class memory_manager
        private:
            dynarray<memory_descriptor> descriptors;
            
            template<class Allocator>
            static dynarray<memory_descriptor>
            convert_from_uefi( UEFI::memory_map &uefi_map, Allocator &&alloc )
            {
                // First simply convert every UEFI descriptor to a usable
                // memory descriptor
                dynarray<memory_descriptor> stage1_descriptors( uefi_map.cbegin_v1(),
                                                               uefi_map.cend_v1(),
                                                               std::forward<Allocator>( alloc ) );
                
                // Now remedy any possible overlaps
                std::sort( )
            }
        public:
            static memory_request maximum_conversion_requirement( UEFI::memory_map &uefi_map )
            {
                return memory_request{ uefi_map.number_of_descriptors * sizeof(memory_descriptor),
                    alignof(memory_descriptor) };
            }
            
            memory_request copy_requirement( void ) const
            {
                return memory_request{ descriptors.size() * sizeof(memory_descriptor),
                    alignof(memory_descriptor) };
            }
            
            template<class Allocator>
            memory_map( UEFI::memory_map &uefi_map, Allocator &&alloc )
            : descriptors( convert_from_uefi( uefi_map, std::forward<Allocator>( alloc ) ) ) {}
        };
    }
}

#endif
