#/** \ingroup kernel
* \{
*
* \file kernel/memory_map.hpp
* \brief This file defines the memory map used by the
*        kernel as well as some auxilliary types.
*/

#ifndef H_kernel_memory
#define H_kernel_memory

#include <utils/dynarray.hpp>
#include <utils/debug.hpp>

#include "constants.hpp"

#include <memory_resource>
#include <algorithm>
#include <limits>
#include <functional>

namespace UtopiaOS
{
    namespace kernel
    {
        /** \enum memory_type
         * \brief Analogous to the UEFI memory type
         *        but contains only types the kernel
         *        actually knows about.
         */
        enum class memory_type : std::uint32_t
        {
            general_purpose,
            unusable,
            invalid
        };
        
        /** \struct memory_descriptor
         * \brief Analogous to a UEFI memory descriptor,
         *        but usable by the kernel. It also has
         *        a well-defined size.
         *
         * It is guaranteed that for a valid memory descriptor:
         * - (\a start + \a number_of_pages * \a pagesize)
         *   does not overflow, where \a start is either
         *   \a physical_start or \a virtual_start.
         * - \a number_of_pages is non-zero
         * For a memory_descriptor with its type set to
         * \a memory_type::invalid the contents of the
         * other fields are undefined.
         */
        struct memory_descriptor
        {
            memory_type type;
            std::uintptr_t physical_start;
            std::uintptr_t virtual_start;
            std::size_t number_of_pages;
            
            /** \brief Returns an invalid memory descriptor.
             * \returns An invalid memory descriptor
             */
            static memory_descriptor invalid_memory_descriptor( void )
            {
                return memory_descriptor{ memory_type::invalid,
                    0, 0, 0 };
            }
            
            /** \brief Constructs a memory descriptor from
             *         properties describing a memory region.
             * \param[in] t The type of the memory region
             * \param[in] ps The physical start of the memory region
             * \param[in] vs The virtual start of the memory region
             * \param[in] np The number of pages of the memory region
             *
             * If the parameters do not fulfil the guarantees
             * required of a memory_descriptor object, a
             * std::invalid_argument exception is thrown.
             */
            memory_descriptor( memory_type t, std::uintptr_t ps,
                              std::uintptr_t vs, std::size_t np )
            : type( t ), physical_start( ps ), virtual_start( vs ),
            number_of_pages( np )
            {
                if( validate( *this ) == false )
                {
                    type = memory_type::invalid;
                    throw std::invalid_argument( "Cannot construct a \
valid memory descriptor with the specified arguments" );
                }
            }
            
            /** \brief Construct a kernel-usable memory
             *         descriptor from a UEFI one.
             *
             * \warning If there is an overflow in
             *          (\a start + \a number_of_pages * \a UEFI::pagesize)
             *          where \a start is either
             *          \a physical_start or \a virtual_start
             *          the memory_type will be set to unusable.
             * \warning If the total memory block is smaller than
             *          the kernel pagesize the type will be set
             *          to invalid and the other fields are undefined.
             * \warning If the kernel pagesize cannot be used to fully
             *          cover the memory region, the memory region
             *          will be truncated accordingly.
             */
            memory_descriptor( const UEFI::memory_descriptor_v1 &uefi_desc )
            : type( uefi_desc.type == UEFI::memory_type::EfiConventionalMemory ?
                   memory_type::general_purpose : memory_type::unusable ),
            physical_start( uefi_desc.physical_start ),
            virtual_start( uefi_desc.virtual_start ),
            number_of_pages( (uefi_desc.number_of_pages * UEFI::pagesize) /
                               pagesize )
            {
                static_assert( std::numeric_limits<std::uintptr_t>::max() >=
                              std::numeric_limits<UEFI::uint64>::max(),
                              "This implementation cannot guarantee to work." );
                static_assert( std::numeric_limits<std::size_t>::max() >=
                              std::numeric_limits<UEFI::uint64>::max(),
                              "This implementation cannot guarantee to work." );
                
                if( validate( *this ) == false )
                    type = memory_type::invalid;
            }
            
            /** \brief Checks whether the given memory region is
             *         contained in the memory described by the
             *         descriptor.
             * \param[in] region The memory region
             * \returns \a true if it is contained, false otherwise
             */
            bool contains_memory_region( const target::memory_region &region ) const
            {
                /** \todo static_assert interger requirements */
                return (region.base() >= virtual_start &&
                        region.top() <= virtual_start + pagesize * number_of_pages);
            }
            
            /** \brief Checks whether the memory descriptor
             *         is valid.
             * \returns \a false if type is memory_type::invalid,
             *          true otherwise.
             */
            bool is_valid( void ) const { return (type != memory_type::invalid); }
        private:
            /** \brief Checks whether a memory descriptor does
             *         not fulfil the required guarantees.
             * \param[in] md The memory descriptor to validate
             * \returns \a true if the memory descriptor fulfils
             *          the guarantees and \a false otherwise.
             */
            static bool validate( const memory_descriptor &md )
            {
                // Check if memory block size already overflows
                if( (std::numeric_limits<std::uintptr_t>::max() /
                     pagesize ) < md.number_of_pages )
                    return false;
                // Check if the block starting at physical_address
                // overflows
                if( (std::numeric_limits<std::uintptr_t>::max() -
                    pagesize * md.number_of_pages) < md.physical_start )
                    return false;
                // Check if the block starting at virtual_address
                // overflows
                if( (std::numeric_limits<std::uintptr_t>::max() -
                    pagesize * md.number_of_pages) < md.virtual_start )
                    return false;
                // Do not retain zero sized memory regions
                if( md.number_of_pages == 0 )
                    return false;
                
                return true;
            }
        };
        
        /** \class memory_map
         * \brief The memory map used by the kernel
         * \tparam Allocator The type of allocator to use
         *
         * It has some sanity guarantees that UEFI lacks, like:
         * - fixed size of descriptors that is known compile-time
         * - overlapping ranges are merged if possible
         *   and removed otherwise
         * - The descriptors are sorted by virtual_start in
         *   ascending order
         */
        template<class Allocator>
        class memory_map
        {
        public:
            using allocator_type = Allocator;
        private:
            using desc_array = utils::dynarray<memory_descriptor, allocator_type>;
            desc_array descriptors;
            
            /** \brief Checks whether the memory regions
             *         described by two memory descriptors
             *         overlap.
             * \param[in] md1 The first memory descriptor
             * \param[in] md2 The second memory descriptor
             * \returns \a true if there is an overlap
             * \warning The virtual start of \a md1 has to
             *          preceede that of \a md2, otherwise
             *          the behaviour is undefined.
             */
            static bool have_overlap( const memory_descriptor &md1,
                        const memory_descriptor &md2 )
            {
                utils::debug_assert( md1.virtual_start <= md2.virtual_start,
                             "md1 does not preceede md2!" );
                
                auto end1 = md1.virtual_start + pagesize * md1.number_of_pages;
                return (end1 > md2.virtual_start);
            };
            
            /** \brief Merges the memory regions described
             *         by two overlapping memory descriptors.
             * \param[in] md1 The first memory descriptor
             * \param[in] md2 The second memory descriptor
             * \returns The merged memory descriptor describing
             *          both memory regions. If the regions
             *          are incompatible or either is invalid
             *          the returned descriptor will also be invalid.
             * \warning The virtual start of \a md1 has to
             *          preceede that of \a md2, otherwise
             *          the behaviour is undefined.
             * \warning The memory regions described by \a md1
             *          and \a md2 have to overlap, otherwise
             *          the behaviour is undefined.
             */
            static memory_descriptor merge_with_overlap( const memory_descriptor &md1,
                                                       const memory_descriptor &md2 )
            {
                utils::debug_assert( md1.virtual_start <= md2.virtual_start,
                             "md1 does not preceede md2!" );
                utils::debug_assert( have_overlap( md1, md2 ),
                             "md1 and md2 do not overlap" );
                
                if( md1.type != md2.type )
                {
                    // Different memory descriptor types
                    // for overlapping ranges... Corrupt!
                    return memory_descriptor::invalid_memory_descriptor();
                }
                
                auto size_before_overlap = md2.virtual_start - md1.virtual_start;
                auto physical_start2 = md1.physical_start + size_before_overlap;
                
                if( md2.physical_start != physical_start2 )
                {
                    // Same virtual address is mapped to multiple
                    // physical addresses... corrupt!
                    return memory_descriptor::invalid_memory_descriptor();
                }
                
                // Legal overlap
                auto end1 = (md1.virtual_start +
                             pagesize * md1.number_of_pages);
                auto end2 = (md2.virtual_start +
                             pagesize * md2.number_of_pages);
                auto end = std::max( end1, end2 );
                
                return memory_descriptor{ md1.type, md1.physical_start,
                    md1.virtual_start, (end - md1.virtual_start) / pagesize };
            }
            
            /** \brief Checks whether the memory regions described
             *         by two memory descriptors are adjacent
             *         and if they are mergable.
             * \param[in] md1 The first memory descriptor
             * \param[in] md2 The second memory descriptor
             * \returns \a true if the ranges are adjacent
             * \warning The virtual start of \a md1 has to
             *          preceede that of \a md2, otherwise
             *          the behaviour is undefined.
             *
             * Two memory ranges are mergable if they share the
             * same type and their physical addresses line up.
             */
            static bool are_adjacent_and_mergable( const memory_descriptor &md1,
                                                  const memory_descriptor &md2 )
            {
                utils::debug_assert( md1.virtual_start <= md2.virtual_start,
                             "md1 does not preceede md2!" );
                
                if( md1.type != md2.type )
                    return false;
                
                auto end1 = md1.virtual_start + pagesize * md1.number_of_pages;
                
                if( md1.physical_start + (end1 - md1.virtual_start) != md2.physical_start )
                    return false;
                
                return (end1 == md2.virtual_start);
            };
            
            /** \brief Merges the memory regions described
             *         by two adjacent memory descriptors.
             * \param[in] md1 The first memory descriptor
             * \param[in] md2 The second memory descriptor
             * \returns The merged memory descriptor describing
             *          both memory regions.
             * \warning The virtual start of \a md1 has to
             *          preceede that of \a md2, otherwise
             *          the behaviour is undefined.
             * \warning The memory regions described by \a md1
             *          and \a md2 have to be adjacent and mergable,
             *          otherwise the behaviour is undefined.
             */
            static memory_descriptor merge_adjacent( const memory_descriptor &md1,
                                                    const memory_descriptor &md2 )
            {
                utils::debug_assert( md1.virtual_start <= md2.virtual_start,
                             "md1 does not preceede md2!" );
                utils::debug_assert( are_adjacent_and_mergable( md1, md2 ),
                             "md1 and md2 are not adjacent!" );
                
                auto end = (md2.virtual_start +
                            pagesize * md2.number_of_pages);
                
                return memory_descriptor{ md1.type, md1.physical_start,
                    md1.virtual_start, (end - md1.virtual_start) / pagesize };
            }
            
            /** \brief Convert a uefi memory map to a
             *         kernel-usable one.
             * \param[in] uefi_map The UEFI memory map
             * \param[in] The allocator to use
             * \returns An array of descriptors with the guarantee
             *          that they do not overlap.
             */
            static desc_array
            convert_from_uefi( const UEFI::memory_map &uefi_map,
                              allocator_type &&alloc )
            {
                desc_array stage1_descriptors( uefi_map.cbegin_v1(),
                                              uefi_map.cend_v1(),
                                              std::forward<allocator_type>( alloc ) );
                
                auto endValid = std::partition( stage1_descriptors.begin(),
                                               stage1_descriptors.end(),
                                               std::mem_fn( &memory_descriptor::is_valid ) );
                
                std::sort( stage1_descriptors.begin(), stage1_descriptors.end(),
                          [] ( const memory_descriptor &md1, const memory_descriptor &md2 ) {
                              return (md1.virtual_start < md2.virtual_start);
                          } );
                
                for( auto lower = stage1_descriptors.begin();
                    lower != endValid; ++lower )
                {
                    auto &md1 = *lower;
                    auto upper = lower + 1;
                    
                    if( upper == endValid )
                        break;
                    auto &md2 = *upper;
                    
                    if( have_overlap( md1, md2 ) )
                    {
                        md2 = merge_with_overlap( md1, md2 );
                        if( md2.type == memory_type::invalid )
                            md1.type = memory_type::invalid;
                        ++lower;
                    } else if( are_adjacent_and_mergable( md1, md2 ) )
                    {
                        md2 = merge_adjacent( md1, md2 );
                        ++lower;
                    }
                }
                
                endValid = std::partition( stage1_descriptors.begin(),
                                          stage1_descriptors.end(),
                                          std::mem_fn( &memory_descriptor::is_valid ) );
                
                return desc_array( std::move( stage1_descriptors ),
                                  endValid - stage1_descriptors.begin() );
            }
        public:
            /** \brief Returns a memory_request that when fulfilled
             *         will suffice to convert a UEFI memory map
             *         to a kernel-usable one.
             * \param[in] uefi_map The UEFI memory map
             * \returns The memory_request
             */
            static target::memory_request<alignof(memory_descriptor)>
            maximum_conversion_requirement( const UEFI::memory_map &uefi_map )
            {
                return { uefi_map.number_of_descriptors * sizeof(memory_descriptor) };
            }
            
            /** \brief Returns a memory_request that when fulfilled
             *         will suffice to copy the memory map.
             * \returns The memory_request
             */
            target::memory_request<alignof(memory_descriptor)> maximum_copy_requirement( void ) const
            {
                return { descriptors.size() * sizeof(memory_descriptor) };
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
            
            template<class OtherAllocator>
            memory_map( const memory_map<OtherAllocator> &other, allocator_type &&alloc )
            : descriptors( other.cbegin(), other.cend(), std::forward<allocator_type>( alloc ) )
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
