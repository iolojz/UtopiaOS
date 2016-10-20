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
#include <array>

#include <boost/hana.hpp>
#include <boost/hana/ext/std/array.hpp>
#include <boost/hana/ext/std/integer_sequence.hpp>

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
            /** \name Memory Tags
             * \brief These type tags represent the different
             *        categories in which the memory manager
             *        divides the total memory it manages.
             * \{
             */
            class memmap_memory_tag {};
            class omd_memory_tag {};
            class avm_memory_tag {};
            class mr_memory_tag {};
            /** \} */
            
            /** \brief A tuple containing the <em> Memory Tags </em>
             * The mr_memory_tag has to be the last one (compile-time enforced).
             */
            static constexpr auto memory_tags = boost::hana::tuple_t<memmap_memory_tag,
                                                       omd_memory_tag,
                                                       avm_memory_tag,
                                                       mr_memory_tag>;
            
            /** \brief A map specifying the order of the <em> Memory Tags </em>
             *         in the \a memory_tags tuple
             */
            static constexpr auto tag_index = boost::hana::unpack(
                boost::hana::zip_with( boost::hana::make_pair,
                                      memory_tags,
                  boost::hana::to_tuple( std::make_index_sequence<boost::hana::length( memory_tags )>() )
                                      ),
                boost::hana::make_map );
            
            static_assert( tag_index[boost::hana::type_c<mr_memory_tag>] ==
                          boost::hana::length( memory_tags ) - boost::hana::size_c<1>,
                          "The mr_memory_tag must be the last tag!" );
            
            /** \brief Same as \a memory_tags but with the mr_memory_tag removed */
            static constexpr auto non_mr_tags = boost::hana::remove_at( memory_tags,
                          tag_index[boost::hana::type_c<mr_memory_tag>] );
            
            /** \brief The number of memory request that will be made upon construction
                       of the \a unsynchronized_memory_manager
             */
            static constexpr auto number_of_memory_requests = boost::hana::length( memory_tags );
            
            /** \brief The number of memory resources that need to be created
             *         upon construction of the \a unsynchronized_memory_manager
             */
            static constexpr auto number_of_memory_resources = boost::hana::length( non_mr_tags );
            
            /** \brief The deleter type used for the memory resources */
            using resource_deleter = utils::destruct_deleter<std::pmr::monotonic_buffer_resource>;
            
            /** \brief A smart pointer managing the memory resources */
            using resource_ptr = std::unique_ptr<std::pmr::monotonic_buffer_resource,
                                                                      resource_deleter>;
            
            /** \brief The memory resources used internally by
             * the unsynchronized_memory_manager
             */
            std::array<resource_ptr, boost::hana::length( non_mr_tags )> memory_resources;
            
            /** \name Allocator Types
             * \brief Used by different managment objects that are part of the
             *        unsynchronized_memory_manager object.
             * \{
             */
            using memory_descriptor_allocator = std::pmr::polymorphic_allocator<memory_descriptor>;
            using memory_region_allocator = std::pmr::polymorphic_allocator<target::memory_region>;
            using buffer_allocator = std::pmr::polymorphic_allocator<std::pmr::monotonic_buffer_resource>;
            /* \} */
            
            /** \name Memory Request Types
             * \brief Used upon memory allocation by the <em> Allocator Types </em>
             * \{
             */
            using omd_request = target::memory_request<
                alignof(std::allocator_traits<memory_region_allocator>::value_type)
            >;
            using mr_request = target::memory_request<
                alignof(std::allocator_traits<buffer_allocator>::value_type)
            >;
            using avm_request = target::memory_request<
                alignof(std::allocator_traits<buffer_allocator>::value_type)
            >;
            /** \} */
            
            /** \brief The memory map */
            memory_map<memory_descriptor_allocator> memmap;
            
            /** \brief The occupied memory description.
             * An array specifying which memory regions are occupied.
             * Occupied regions are specified upon construction of
             * the \a unsynchronized_memory_manager object and also
             * contain special memory regions that are reserved for
             * the \a unsynchronized_memory_manager object itself.
             */
            utils::dynarray<target::memory_region, memory_region_allocator> omd;
            
            /** \brief The available memory.
             * An array of allocators that can be used to manage
             * the available memory. Available memory is memory
             * described by the memory map that is considered
             * usable (See \a enumerate_available_memory) and
             * that is not occupied in the sense described by
             * the \a omd.
             */
            utils::dynarray<std::pmr::monotonic_buffer_resource, buffer_allocator> available_memory;
            
            /** \class memory_requirement
             * \brief A Function object returning the memory
             *        requirement specified by a given
             *        memory type tag.
             * \tparam MemMap The memory map type
             * \tparam RandomAccessIterator The omd iterator type
             */
            template<class MemMap, class RandomAccessIterator>
            class memory_requirement
            {
            private:
                const MemMap &memmap; /**< A reference to the memory map */
                const RandomAccessIterator omd_begin; /** Begin of the omd */
                const RandomAccessIterator omd_end; /** End of the omd */
            public:
                /** \brief Construct a memory_requirement object
                 * \param[in] mm The memory map
                 * \param[in] ob The begin of the omd
                 * \param[in] oe The end of the omd
                 */
                memory_requirement( const MemMap &mm,
                                   RandomAccessIterator ob,
                                   RandomAccessIterator oe )
                : memmap( mm ), omd_begin( ob ), omd_end( oe ) {}
                
                /** \brief Overload for \a memmap_memory_tag */
                auto operator()( boost::hana::basic_type<memmap_memory_tag> )
                { return memmap.maximum_copy_requirement(); }
                
                /** \brief Overload for \a omd_memory_tag */
                omd_request operator()( boost::hana::basic_type<omd_memory_tag> )
                {
                    std::size_t number_of_new_omds = number_of_memory_requests;
                    auto min_omds = omd_end - omd_begin;
                    auto max_omds = min_omds + number_of_new_omds;
                    
                    return {sizeof(target::memory_region) * max_omds};
                }
                
                /** \brief Overload for \a mr_memory_tag */
                constexpr mr_request operator()( boost::hana::basic_type<mr_memory_tag> )
                {
                    return { number_of_memory_resources *
                        sizeof(std::pmr::monotonic_buffer_resource) };
                }
                
                /** \brief Overload for \a avm_memory_tag */
                avm_request operator()( boost::hana::basic_type<avm_memory_tag> )
                {
                    auto max_new_avm_fragments = number_of_memory_requests;
                    auto min_avm_fragments = number_of_avm_fragments( memmap,
                                                                     omd_begin,
                                                                     omd_end );
                    auto max_fragments = max_new_avm_fragments + min_avm_fragments;
                    
                    return { max_fragments *
                        sizeof(std::pmr::monotonic_buffer_resource) };
                }
            };
            
            /** \brief Helper function that constructs a
             *         memory_requirement object.
             * \tparam MemMap The memory map type (usually inferred)
             * \tparam RandomAccessIterator The omd iterator type (usually inferred)
             * \param[in] memmap The memory map
             * \param[in] omd_begin The begin of the omd
             * \param[in] omd_end The end of the omd
             */
            template<class MemMap, class RandomAccessIterator>
            memory_requirement<MemMap, RandomAccessIterator>
            static get_memory_requirement( const MemMap &memmap,
                                          RandomAccessIterator omd_begin,
                                          RandomAccessIterator omd_end )
            { return memory_requirement<MemMap, RandomAccessIterator>( memmap,
                                                                      omd_begin,
                                                                      omd_end ); }
            
            /** \brief Builds the memory manager step by step.
             * \tparam MemMap A kernel-usable memory map type
             * \tparam RandomAccessIterator An iterator type
             *         whose value_types are memory_regions.
             * \param[in] memmap The memory map
             * \param[in] omd_begin Begin of omd
             * \param[in] omd_end End of omd
             * 
             * The omd range has to be sorted in ascending order.
             */
            template<class MemMap, class RandomAccessIterator>
            static unsynchronized_memory_manager build_memory_manager( const MemMap &memmap,
                                                                      RandomAccessIterator omd_begin,
                                                                      RandomAccessIterator omd_end )
            {
                namespace hana = boost::hana;
                
                utils::debug_assert( std::is_sorted( omd_begin, omd_end ),
                             "The omd has to be sorted!" );
                
                // Sanity check: Is all occupied memory contained in the memory map?
                std::for_each( omd_begin, omd_end, [&memmap] ( const target::memory_region &region ) {
                    auto it = std::find_if( memmap.cbegin(), memmap.cend(),
                           std::bind( std::mem_fn( &memory_descriptor::contains_memory_region ),
                                     std::placeholders::_1, region ) );
                    
                    if( it == memmap.cend() )
                        throw std::invalid_argument( "Occupied memory not contained in memory map" );
                } );
                
                // Get the memory request for every memory tag
                auto memory_requests = hana::transform( memory_tags,
                                        get_memory_requirement( memmap, omd_begin, omd_end ) );
                
                // Allocate space for storing the memory regions that
                // are used to meet the above requests.
                auto request_omds = hana::replicate<hana::tuple_tag>( target::memory_region(),
                                                                     hana::length( memory_tags ) );
                
                // Allocate the space requested in the \a memory_requests
                // above and calculate the new omd, that marks these
                // allocated regions as occupied.
                auto new_omd = hana::fold_left( memory_tags,
                                               std::make_pair( omd_begin, omd_end ),
                    [&] ( const auto &state, auto tag ) {
                        const auto &omd_begin = state.first;
                        const auto &omd_end = state.second;
                        const auto &request = memory_requests[tag_index[tag]];
                        
                        request_omds[tag_index[tag]] = meet_request( memmap,
                                                                    omd_begin,
                                                                    omd_end,
                                                                    request );
                        return utils::sorted_range_insert_reference( omd_begin, omd_end,
                                                             request_omds[tag_index[tag]] );
                    }
                );
                
                auto mr_omd = request_omds[tag_index[hana::type_c<mr_memory_tag>]];
                
                // Get the space designated to store the memory resource objects
                auto mr_memory = target::uintptr_to_ptr<
                    std::allocator_traits<buffer_allocator>::pointer
                >( mr_omd.base() );
                
                // Put the memory resource objects into place
                auto memory_resources = boost::hana::transform( non_mr_tags,
                          [&] ( auto tag ) {
                    const auto &region = request_omds[tag_index[tag]];
                    return resource_ptr( new (mr_memory + tag_index[tag])
                            std::allocator_traits<buffer_allocator>::value_type( region.base_ptr(),
                                                                                region.size ) );
                } );
                
                return unsynchronized_memory_manager( memmap,
                                                     new_omd.first,
                                                     new_omd.second,
                                                     std::move( memory_resources ) );
            }
            
            /** \brief Try to fulfil a memory request
             * \tparam MemMap The memory map type
             * \tparam InputIterator The omd iterator type
             * \tparam MemRequest The memory request type
             * \param[in] memmap The memory map
             * \param[in] omd_begin The begin of the omd
             * \param[in] omd_end The end of the omd
             * \param[in] request The memory request to fulfil
             *
             * \note The omd range has to be sorted.
             * \note Throws a \ std::runtime_error if the request
             * could not be fulfilled.
             *
             * \returns A memory region that fulfils the request
             */
            template<class MemMap, class InputIterator, class MemRequest>
            static target::memory_region meet_request( const MemMap &memmap,
                                                      InputIterator omd_begin,
                                                      InputIterator omd_end,
                                                      const MemRequest &request )
            {
                for( auto desc_it = memmap.cbegin(); desc_it != memmap.cend(); ++desc_it )
                {
                    const auto &desc = *desc_it;
                    
                    bool success = true;
                    
                    // Try the lowest possible region within the memory descriptor first
                    auto aligned_address = target::align<MemRequest::alignment>( desc.virtual_start );
                    target::memory_region attempt{ aligned_address, aligned_address + request.size };
                    
                    if( desc.contains_memory_region( attempt ) == false )
                        continue;
                    
                    // Successively try higher regions if the attempted region
                    // and an occupied region intersect.
                    auto intersection = omd_begin;
                    while( (intersection = std::find_if( intersection, omd_end,
                                   std::bind( &target::memory_region::intersects_memory_region,
                                   attempt,
                                   std::placeholders::_1 ) )) != omd_end )
                    {
                        aligned_address = target::align<MemRequest::alignment>( intersection->top() );
                        attempt = { aligned_address, aligned_address + request.size };
                        
                        if( desc.contains_memory_region( attempt ) == false )
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
            
            /** \brief Calculate the number of memory fragments
             * \tparam MemMap The memory map type
             * \tparam InputIterator The omd iterator type
             * \param[in] memmap The memory map
             * \param[in] omd_begin The begin of the omd
             * \param[in] omd_end The end of the omd
             *
             * \returns The number of non-zero memory fragments.
             */
            template<class MemMap, class InputIterator>
            static std::size_t
            number_of_avm_fragments( const MemMap &memmap,
                                    InputIterator omd_begin,
                                    InputIterator omd_end );
            
            /** \brief Calculate the number of memory fragments
             * \tparam MemMap The memory map type
             * \tparam InputIterator The omd iterator type
             * \param[in] memmap The memory map
             * \param[in] omd_begin The begin of the omd
             * \param[in] omd_end The end of the omd
             * \param[in] alloc The allocator to be used for the
             *            construction of the return value.
             *
             * \returns An array of \a std::memory_resource
             *          objects that cover the available memory
             *
             * \note \a alloc has to be able to allocate
             *       at least \a number_of_avm_fragments()
             *       memory_resource objects otherwise
             *       the behaviour is undefined.
             */
            template<class MemMap, class InputIterator>
            static utils::dynarray<std::pmr::monotonic_buffer_resource, buffer_allocator>
            enumerate_avm( const MemMap &memmap,
                          InputIterator omd_begin,
                          InputIterator omd_end,
                          buffer_allocator &&alloc );
            
            /** \brief Construct an \a unsynchronized_memory_manager from
             *         data computed by \a build_memory_manager()
             * \tparam MemMap The memory map type
             * \tparam InputIterator The omd iterator type
             * \tparam Resources The memory resource holding type
             * \param[in] mm The memory map
             * \param[in] omd_begin The begin of the omd
             * \param[in] omd_end The end of the omd
             * \param[inout] mr The object holding the memory resource
             *                  objects that are to be use for internal
             *                  allocations.
             *
             * \note The resources in \a mr will always be stolen (moved).
             */
            template<class MemMap, class InputIterator, class Resources>
            unsynchronized_memory_manager( const MemMap &mm,
                                          InputIterator omd_begin,
                                          InputIterator omd_end,
                                          Resources &&mr )
            : memory_resources( std::move( mr ) ),
            memmap( mm,
                   memory_resources[tag_index[boost::hana::type_c<memmap_memory_tag>]].get() ),
            omd( omd_begin, omd_end,
                memory_resources[tag_index[boost::hana::type_c<omd_memory_tag>]].get() ),
            available_memory( enumerate_avm( memmap, omd.cbegin(), omd.cend(),
                 memory_resources[tag_index[boost::hana::type_c<avm_memory_tag>]].get() ) )
            {}
        public:
            /** \brief Construct the memory manager from a memory map
             *         and data about occupied memory. omd is short
             *         for "occupied memory description".
             * \tparam MemMap A kernel-usable memory map type
             * \tparam RandomAccessIterator An iterator type whose value_types are
             *                              memory_regions.
             * \param[in] mm The old memory map
             * \param[in] omd_begin Begin of omd
             * \param[in] omd_end End of omd
             */
            template<class MemMap, class RandomAccessIterator>
            unsynchronized_memory_manager( const MemMap &mm,
                                          RandomAccessIterator omd_begin,
                                          RandomAccessIterator omd_end )
            : unsynchronized_memory_manager( build_memory_manager( mm, omd_begin, omd_end ) )
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
