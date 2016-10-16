#/** \ingroup kernel
* \{
*
* \file kernel/memory_manager.hpp
* \brief This file declares two types of memory managers,
*        an unsynchronized and a synchronized.
*
* They are intended to be used during different stages of
* OS boot.
*/

#ifndef H_kernel_memory_manager
#define H_kernel_memory_manager

#include "memory_map.hpp"

#include <utils/debug.hpp>
#include <utils/destruct_deleter.hpp>
#include <utils/ranges.hpp>

#include <memory>
#include <algorithm>
#include <memory_resource>
#include <functional>

namespace UtopiaOS
{
    namespace kernel
    {
        /** \class unsynchronized_memory_manager
         * \brief A memory managing object from which allocators
         *        can be retrieved.
         */
        class unsynchronized_memory_manager
        {
        private:
            using resource_deleter = utils::destruct_deleter<std::pmr::monotonic_buffer_resource>;
            using resource_ptr = std::unique_ptr<std::pmr::monotonic_buffer_resource,
                                                                      resource_deleter>;
            
            resource_ptr &&memmap_resource;
            resource_ptr &&omd_resource;
            resource_ptr &&avm_resource;
            
            using memory_descriptor_allocator = std::pmr::polymorphic_allocator<memory_descriptor>;
            using memory_region_allocator = std::pmr::polymorphic_allocator<target::memory_region>;
            using buffer_allocator = std::pmr::polymorphic_allocator<std::pmr::monotonic_buffer_resource>;
            
            memory_map<memory_descriptor_allocator> memmap;
            utils::dynarray<target::memory_region, memory_region_allocator> omd;
            utils::dynarray<std::pmr::monotonic_buffer_resource, buffer_allocator> available_memory;
            
            /** \brief Builds the memory manager step by step.
             * \tparam MemMap A kernel-usable memory map type
             * \tparam RandomAccessIterator An iterator type
             *         whose value_types are memory_regions.
             * \param[in] old_memmap The old memory map
             * \param[in] omd_begin Begin of omd
             * \param[in] omd_end End of omd
             * 
             * The omd range has to be sorted in ascending order.
             */
            template<class MemMap, class RandomAccessIterator>
            static unsynchronized_memory_manager build_memory_manager( MemMap &&old_memmap,
                                                                      RandomAccessIterator omd_begin,
                                                                      RandomAccessIterator omd_end )
            {
                std::for_each( omd_begin, omd_end, [&old_memmap] ( const target::memory_region &region ) {
                    auto it = std::find_if( old_memmap.cbegin(), old_memmap.cend(),
                           std::bind( std::mem_fn( &memory_descriptor::contains_memory_region ),
                                     std::placeholders::_1, region ) );
                    
                    if( it == old_memmap.cend() )
                        throw std::invalid_argument( "Occupied memory not contained in memory map" );
                } );
                
                const auto memmap_requirement = old_memmap.maximum_copy_requirement();
                target::memory_region memmap_omd = meet_request( old_memmap,
                                                                omd_begin,
                                                                omd_end,
                                                                memmap_requirement );
                
                const auto omd_stage2 = utils::sorted_range_insert_reference( omd_begin,
                                                                             omd_end,
                                                                             memmap_omd );
                
                // We need three more omds at this point
                using omd_request = target::memory_request<alignof(target::memory_region)>;
                const omd_request omd_requirement = {sizeof(target::memory_region) *
                    (3 + (omd_stage2.second - omd_stage2.first))};
                target::memory_region omd_omd = meet_request( old_memmap,
                                                             omd_stage2.first,
                                                             omd_stage2.second,
                                                             omd_requirement );
                
                const auto omd_stage3 = utils::sorted_range_insert_reference( omd_stage2.first,
                                                                             omd_stage2.second,
                                                                             omd_omd );
                
                // We introduce the abbreviation "mr" for memory resource
                using mr_request = target::memory_request<alignof(std::pmr::monotonic_buffer_resource)>;
                const mr_request mr_requirement = { 3 *
                    sizeof(std::pmr::monotonic_buffer_resource) };
                target::memory_region mr_omd = meet_request( old_memmap,
                                                            omd_stage3.first,
                                                            omd_stage3.second,
                                                            mr_requirement );
                
                const auto omd_stage4 = utils::sorted_range_insert_reference( omd_stage3.first,
                                                                             omd_stage3.second,
                                                                             mr_omd );
                
                // In the worst case we get one more fragments for one more omd
                auto max_fragments = 1 + number_of_available_memory_fragments( old_memmap,
                                                                              omd_stage4.first,
                                                                              omd_stage4.second );
                
                // We introduce the abbreviation "avm" for available memory
                using avm_request = target::memory_request<alignof(std::pmr::monotonic_buffer_resource)>;
                const avm_request avm_requirement = { max_fragments *
                    sizeof(std::pmr::monotonic_buffer_resource) };
                target::memory_region avm_omd = meet_request( old_memmap,
                                                             omd_stage4.first,
                                                             omd_stage4.second,
                                                             avm_requirement );
                
                const auto omd_stage5 = utils::sorted_range_insert_reference( omd_stage4.first,
                                                                             omd_stage4.second,
                                                                             avm_omd );
                
                // Now we have all the memory we need!
                auto memory_resources =
                  target::uintptr_to_ptr<std::pmr::monotonic_buffer_resource*>( mr_omd.base() );
                
                resource_ptr memmap_resource( new (memory_resources)
                                             std::pmr::monotonic_buffer_resource( memmap_omd.base_ptr(),
                                                                                 memmap_omd.size ) );
                resource_ptr omd_resource( new (memory_resources + 1)
                                          std::pmr::monotonic_buffer_resource( omd_omd.base_ptr(),
                                                                              omd_omd.size ) );
                resource_ptr avm_resource( new (memory_resources + 2)
                                          std::pmr::monotonic_buffer_resource( avm_omd.base_ptr(),
                                                                              avm_omd.size ) );
                
                return unsynchronized_memory_manager( std::forward<MemMap>( old_memmap ),
                                                     omd_stage5.first,
                                                     omd_stage5.second,
                                                     std::move( memmap_resource ),
                                                     std::move( omd_resource ),
                                                     std::move( avm_resource ) );
            }
            
            template<class MemMap, class InputIterator, class MemRequest>
            static target::memory_region meet_request( const MemMap &memmap,
                                                      InputIterator omd_begin,
                                                      InputIterator omd_end,
                                                      const MemRequest &request )
            {
                for( auto desc_it = memmap.cbegin(); desc_it != memmap.cend(); ++desc_it )
                {
                    bool success = true;
                    auto aligned_address = target::align<request.alignment>( desc_it->virtual_start );
                    target::memory_region attempt{ aligned_address, aligned_address + request.size };
                    
                    if( desc_it->contains_memory_region( attempt ) == false )
                        continue;
                    
                    auto intersection = omd_begin;
                    while( (intersection = std::find_if( intersection, omd_end,
                                   std::bind( &target::memory_region::intersects_memory_region,
                                   attempt,
                                   std::placeholders::_1 ) )) != omd_end )
                    {
                        aligned_address = target::align<request.alignment>( intersection->top() );
                        attempt = { aligned_address, aligned_address + request.size };
                        
                        if( desc_it->contains_memory_region( attempt ) == false )
                        {
                            success = false;
                            break;
                        }
                        
                        ++intersection;
                    }
                    
                    if( success )
                        return attempt;
                }
                
                throw std::runtime_error( "Cannot meet memory request" );
            }
            
            template<class MemMap, class InputIterator>
            static unsigned
            number_of_available_memory_fragments( const MemMap &memmap,
                                                 InputIterator omd_begin,
                                                 InputIterator omd_end );
            
            template<class MemMap, class InputIterator, class Allocator>
            static utils::dynarray<std::pmr::monotonic_buffer_resource, Allocator>
            enumerate_avm( const MemMap &memmap,
                          InputIterator omd_begin,
                          InputIterator omd_end,
                          Allocator &&alloc );
            
            template<class MemMap, class InputIterator>
            unsynchronized_memory_manager( const MemMap &old_memmap,
                                          InputIterator omd_begin,
                                          InputIterator omd_end,
                                          resource_ptr &&memmap_res,
                                          resource_ptr &&omd_res,
                                          resource_ptr &&avm_res )
            : memmap_resource( std::move( memmap_res ) ),
            omd_resource( std::move( omd_res ) ),
            avm_resource( std::move( avm_res ) ),
            memmap( old_memmap, memmap_resource.get() ),
            omd( omd_begin, omd_end, omd_resource.get() ),
            available_memory( enumerate_avm( memmap,
                                            omd.cbegin(),
                                            omd.cend(),
                                            buffer_allocator( avm_resource.get() ) ) )
            {}
        public:
            /** \brief Construct the memory manager from a memory map
             *         and data about occupied memory. omd is short
             *         for "occupied memory description".
             * \tparam MemMap A kernel-usable memory map type
             * \tparam InputIterator An iterator type whose value_types are
             *         memory_regions.
             * \param[in] old_memmap The old memory map
             * \param[in] omd_begin Begin of omd
             * \param[in] omd_end End of omd
             */
            template<class MemMap, class RandomAccessIterator>
            unsynchronized_memory_manager( MemMap &&old_memmap,
                                          RandomAccessIterator &&omd_begin,
                                          RandomAccessIterator &&omd_end )
            : unsynchronized_memory_manager(
                    build_memory_manager( std::forward<MemMap>( old_memmap ),
                                         std::forward<RandomAccessIterator>( omd_begin ),
                                         std::forward<RandomAccessIterator>( omd_end ) ) )
            {}
            
            /** \brief Because an unsynchronized_memory_manager manages a single
             *         std::pmr::unsynchronized_pool_resource which is itself
             *         not copyable, the copy constructor has to be deleted.
             */
            unsynchronized_memory_manager( const unsynchronized_memory_manager & ) = delete;
            
            /** \brief An unsynchronized_memory_manager is explicitly
             *         move-constructible!
             */
            unsynchronized_memory_manager( unsynchronized_memory_manager && ) = default;
            
            /** \brief Because an unsynchronized_memory_manager manages a single
             *         std::pmr::unsynchronized_pool_resource which is itself
             *         not copy-assignable, the copy-assign constructor has to be deleted.
             */
            unsynchronized_memory_manager &operator=( const unsynchronized_memory_manager & ) = delete;
            
            /** \brief An unsynchronized_memory_manager is explicitly
             *         move-assignable!
             */
            unsynchronized_memory_manager &operator=( unsynchronized_memory_manager && ) = default;
        };
    }
}

#endif

/** \} */
