#/** \ingroup kernel
* \{
*
* \file kernel/memory_map.hpp
* \brief This file defines the memory map used by the
*        kernel as well as some auxilliary types.
*/

#ifndef H_kernel_memory
#define H_kernel_memory

#include <common/types.hpp>

#include "dynarray.hpp"
#include "debug.hpp"
#include "constants.hpp"

#include <memory_resource>
#include <algorithm>

namespace UtopiaOS
{
    namespace kernel
    {
        /** \enum memory_type
         * \brief Analogous to the UEFI memory type
         *        but contains only types the kernel
         *        actually knows about.
         */
        enum class memory_type : common::uint32
        {
            general_purpose,
            unusable
        };
        
        /** \struct memory_request
         * \brief Encapsulates a memory requirement
         */
        struct memory_request
        {
            common::uint64 size;
            common::un alignment;
        };
        
        /** \brief Aligns a pointer to a given alignment.
         * \param[in] ptr The pointer to align
         * \param[in] alignment The alignment to meet
         * \returns An aligned pointer or 0 if the pointer
         *          could not be aligned.
         *
         * The aligned pointer is guaranteed to point to
         * an address not smaller than the original pointer.
         * It is guaranteed that there is no other aligned
         * address in the range [\a ptr, \a return-value].
         * If the pointer cannot be aligned, i.e it points
         * to an address so large that the next aligned
         * address would not fit into the data type,
         * 0 is returned.
         *
         * \warning If \a alignment is not a power of two,
         *          the behaviour is undefined!
         */
        static inline common::uintptr align( common::uintptr ptr, common::un alignment )
        {
            static_assert( sizeof(common::uintptr) >= sizeof(common::un),
                          "This implementation cannot guarantee to work." );
            debug_assert( (alignment % 2) == 0, "alignment has to be a power of two" );
            
            common::un mask = alignment - 1; // ...0000011111...
            common::un diff = (ptr & mask);
            
            if( diff == 0 )
                return ptr;
            
            return (ptr + alignment - diff);
        }
        
        /** \struct memory_descriptor
         * \brief Analogous to a UEFI memory descriptor,
         *        but usable by the kernel. It also has
         *        a well-defined size.
         */
        struct memory_descriptor
        {
            memory_type type;
            common::uint64 physical_start;
            common::uint64 virtual_start;
            common::uint64 number_of_pages;
            
            /** \brief Construct a kernel-usable memory
             *         descriptor from a UEFI one.
             */
            memory_descriptor( const UEFI::memory_descriptor_v1 &uefi_desc );
            
            /** \brief Checks whether
            bool can_meet_request( const memory_request &request ) const
            {
                static_assert( sizeof(common::uintptr) >= sizeof(common::un),
                              "This implementation cannot guarantee to work." );
                if( type != memory_type::general_purpose )
                    return false;
                
                auto aligned_address = align( virtual_start, request.alignment );
                common::uintptr request_end = aligned_address + request.size;
                
                if( (request_end - virtual_start) / pagesize <= number_of_pages )
                    return true;
                
                return false;
            }
        };
        
        /** \class memory_map
         * \brief The memory map used by the kernel
         * \tparam Allocator The type of allocator to use
         *
         * It has some sanity guarantees that UEFI lacks, like:
         * - fixed size of descriptors that is known compile-time
         * - no overlaps
         * - The memory descriptors always describe regions that
         *   are as large as possible, with lower ones always being
         *   full in a case of ambiguity.
         */
        template<class Allocator>
        class memory_map
        {
        public:
            using allocator_type = Allocator;
        private:
            using desc_array = dynarray<memory_descriptor, allocator_type>;
            desc_array descriptors;
            
            /** \brief Convert a uefi memory map to a
             *         kernel-usable one.
             * \param[in] uefi_map The UEFI memory map
             * \param[in] The allocator to use
             * \returns An array of descriptors with the guarantee
             *          that they do not overlap.
             */
            static desc_array
            convert_from_uefi( const UEFI::memory_map &uefi_map, allocator_type &&alloc )
            {
                desc_array stage1_descriptors( uefi_map.cbegin_v1(),
                                              uefi_map.cend_v1(),
                                              std::forward<allocator_type>( alloc ) );
                
                /** \todo Now remedy all possible overlaps and
                 *        any unnecessary fragmentation
                 */
            }
        public:
            /** \brief Returns a memory_request that when fulfilled
             *         will suffice to convert a UEFI memory map
             *         to a kernel-usable one.
             * \param[in] uefi_map The UEFI memory map
             * \returns The memory_request
             */
            static memory_request maximum_conversion_requirement( const UEFI::memory_map &uefi_map )
            {
                return memory_request{ uefi_map.number_of_descriptors * sizeof(memory_descriptor),
                    alignof(memory_descriptor) };
            }
            
            /** \brief Returns a memory_request that when fulfilled
             *         will suffice to copy the memory map.
             * \returns The memory_request
             */
            memory_request copy_requirement( void ) const
            {
                return memory_request{ descriptors.size() * sizeof(memory_descriptor),
                    alignof(memory_descriptor) };
            }
            
            /** \brief Constructs a kernel-usable memory map
             *         from a UEFI memory map.
             * \param[in] uefi_map The UEFI memory map
             * \param[in] alloc The allocator used to copy the
             *                  memory map. It has to be able to
             *                  allocate at least what is returned
             *                  by \a maximum_conversion_requirement.
             */
            memory_map( const UEFI::memory_map &uefi_map, allocator_type &&alloc )
            : descriptors( convert_from_uefi( uefi_map,
                                             std::forward<allocator_type>( alloc ) ) )
            {}
            
            /** \brief Returns a const iterator to the begin of
             *         the memory descriptor range.
             * \returns A const iterator to the begin of
             *          the memory descriptor range
             */
            typename desc_array::const_iterator cbegin( void ) const
            { return descriptors.cbegin(); }
            
            /** \brief Returns a const iterator to the end of
             *         the memory descriptor range.
             * \returns A const iterator to the end of
             *          the memory descriptor range
             */
            typename desc_array::const_iterator cend( void ) const
            { return descriptors.cend(); }
        };
    }
}

#endif

/** \} */
