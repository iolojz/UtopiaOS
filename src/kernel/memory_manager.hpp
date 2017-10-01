/** \ingroup kernel
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
#include "distributed_resource.hpp"
#include "buddy_resource.hpp"

#include "target/config.hpp"
#include "utils/debug.hpp"
#include "utils/destruct_deleter.hpp"
#include "utils/ranges.hpp"

#ifdef UTOPIAOS_ALLOCA_WITH_ALIGN_HEADER
#include UTOPIAOS_ALLOCA_WITH_ALIGN_HEADER
#endif

#include <memory>
#include <algorithm>
#include <memory_resource>
#include <functional>
#include <array>
#include <iterator>

#include <boost/hana.hpp>
#include <boost/hana/ext/std/integer_sequence.hpp>
#include <boost/iterator/transform_iterator.hpp>

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
            /** \} */
            
            /** \brief A tuple containing the <em> Memory Tags </em>
             * The mr_memory_tag has to be the last one (compile-time enforced).
             */
            static constexpr auto memory_tags = boost::hana::tuple_t<memmap_memory_tag,
                                                       omd_memory_tag,
                                                       avm_memory_tag>;
            
            /** \brief A map specifying the order of the <em> Memory Tags </em>
             *         in the \a memory_tags tuple
             */
            static constexpr auto tag_index = boost::hana::unpack(
                boost::hana::zip_with( boost::hana::make_pair,
                                      memory_tags,
                  boost::hana::to_tuple( std::make_index_sequence<boost::hana::length( memory_tags )>() )
                                      ),
                boost::hana::make_map );
            
            /** \brief The number of memory request that will be made upon construction
                       of the \a unsynchronized_memory_manager
             */
            static constexpr auto number_of_memory_requests = boost::hana::length( memory_tags ) + 1u;
            
            /** \name Internal memory resources
             *  \brief The memory resource types used internally by the \a memory_manager
             *  \{
             */
            static constexpr auto number_of_iresources = boost::hana::length( memory_tags );
            using iresource = std::pmr::monotonic_buffer_resource;
            using iresource_ptr = std::unique_ptr<
                iresource,
                utils::destruct_deleter<iresource>
            >;
            std::array<iresource_ptr, number_of_iresources> iresources;
            /** \} */
            
            /** \brief The memory map */
            memory_map<
                std::pmr::polymorphic_allocator<memory_descriptor>
            > memmap;
            
            /** \brief The occupied memory description.
             * An array specifying which memory regions are occupied.
             * Occupied regions are specified upon construction of
             * the \a unsynchronized_memory_manager object and also
             * contain special memory regions that are reserved for
             * the \a unsynchronized_memory_manager object itself.
             */
            utils::dynarray<
                target::memory_region,
                std::pmr::polymorphic_allocator<target::memory_region>
            > omd;
            
            /** \brief The available memory.
             * An array of allocators that can be used to manage
             * the available memory. Available memory is memory
             * described by the memory map that is considered
             * usable (See \a enumerate_available_memory) and
             * that is not occupied in the sense described by
             * the \a omd.
             */
            utils::dynarray<
                std::pmr::monotonic_buffer_resource,
                std::pmr::polymorphic_allocator<std::pmr::monotonic_buffer_resource>
            > available_memory;
            
            /** \brief The \a std::memory_resource object managing
             *         the available memory.
             */
            distributed_resource avm_resource;
            
            /** \brief A std::pmr::memory_resource object that
             *         is exposed for general purpose allocations
             *         of the available memory.
             * \note It can only allocate sub-pagesize memory
             *       chunks.
             */
            buddy_resource subpage_resource;
            
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
                const RandomAccessIterator omd_begin; /**< Begin of the omd */
                const RandomAccessIterator omd_end; /**< End of the omd */
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
                target::memory_request<
                    alignof(std::allocator_traits<
                                typename decltype(omd)::allocator_type
                            >::value_type)
                > operator()( boost::hana::basic_type<omd_memory_tag> )
                {
                    std::size_t number_of_new_omds = number_of_memory_requests;
                    auto min_omds = omd_end - omd_begin;
                    auto max_omds = min_omds + number_of_new_omds;
                    
                    return {sizeof(typename decltype(omd)::value_type) * max_omds};
                }
                
                /** \brief Overload for \a avm_memory_tag */
                target::memory_request<
                    alignof(std::allocator_traits<
                                typename decltype(available_memory)::allocator_type
                            >::value_type)
                > operator()( boost::hana::basic_type<avm_memory_tag> )
                {
                    auto max_new_avm_regions = number_of_memory_requests;
                    auto min_avm_regions = number_of_avm_regions( memmap,
                                                                 omd_begin,
                                                                 omd_end );
                    auto max_regions = max_new_avm_regions + min_avm_regions;
                    
                    return { max_regions *
                        sizeof(typename decltype(available_memory)::value_type) };
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
             * \note The omd range has to be sorted in ascending
             *       order otherwise the behaviour is undefined.
             * \throws std::invalid_argument if the omd range is
             *         not contained within the memory map.
             */
            template<class MemMap, class RandomAccessIterator>
            static unsynchronized_memory_manager build_memory_manager( const MemMap &memmap,
                                                                      RandomAccessIterator omd_begin,
                                                                      RandomAccessIterator omd_end )
            {
                utils::debug_assert( std::is_sorted( omd_begin, omd_end ),
                             "The omd has to be sorted!" );
                
                namespace hana = boost::hana;
                
                auto omd_view = boost::make_iterator_range( omd_begin, omd_end );
                
                // Sanity check: Is all occupied memory contained in the memory map?
                std::for_each( boost::begin( omd_view ),
                              boost::end( omd_view ),
                              [&memmap] ( const target::memory_region &region ) {
                    auto it = std::find_if( memmap.cbegin(), memmap.cend(),
                                std::bind( std::mem_fn( &memory_descriptor::contains_memory_region ),
                                                        std::placeholders::_1, region ) );
                                  
                    if( it == memmap.cend() )
                        throw std::invalid_argument( "Occupied memory not contained in memory map" );
                } );
                
                // Get the memory requirement for every memory tag
                auto memory_requests = hana::transform( memory_tags,
                                            get_memory_requirement( memmap,
                                                                   boost::begin( omd_view ),
                                                                   boost::end( omd_view ) ) );
                
                // Allocate space for storing the memory regions that
                // are used to meet the above requests.
                auto internal_omds = hana::replicate<hana::tuple_tag>( target::memory_region(),
                                                                     hana::length( memory_tags ) );
                
                // Allocate the space requested in the \a memory_requests
                // above and calculate the new omd, that marks these
                // allocated regions as occupied.
                auto omd_stage2 = hana::fold_left( memory_tags,
                                               omd_view,
                    [&] ( const auto &omd, auto tag ) {
                        const auto &request = memory_requests[tag_index[tag]];
                        
                        internal_omds[tag_index[tag]] = meet_request( memmap,
                                                                     boost::begin( omd ),
                                                                     boost::end( omd ),
                                                                     request );
                        return utils::sorted_range_insert_reference( omd,
                                                             internal_omds[tag_index[tag]] );
                    }
                );
                
                // We also need to store the internal resource objects
                // somewhere.
                target::memory_request<
                    alignof(std::pmr::monotonic_buffer_resource)
                > iresource_request = { hana::length( memory_tags ) *
                              sizeof(std::pmr::monotonic_buffer_resource) };
                auto iresource_omd = meet_request( memmap,
                                                  boost::begin( omd_stage2 ),
                                                  boost::end( omd_stage2 ),
                                                  iresource_request );
                auto omd_final = utils::sorted_range_insert_reference( omd_stage2,
                                                             iresource_omd );
                
                // An array of iresource_ptrs to the internal resource
                // objects.
                std::array<iresource_ptr, hana::length( memory_tags )> iresources;
                
                // Put the internal memory resource objects into place
                hana::for_each( memory_tags, [&] ( auto tag ) {
                    const auto &region = internal_omds[tag_index[tag]];
                    auto base = target::uintptr_to_ptr<
                        std::pmr::monotonic_buffer_resource
                    >( iresource_omd.base() );
                    
                    iresources[tag_index[tag]] =
                        iresource_ptr( new (base + tag_index[tag]) 
                             iresource( region.base_ptr(), region.size ) );
                } );
                
                return unsynchronized_memory_manager( memmap,
                                                     boost::begin( omd_final ),
                                                     boost::end( omd_final ),
                                                     std::move( iresources ) );
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
             * \returns A memory region that fulfils the request,
             *          where \a base() always has the 
             *          requested alignment.
             */
            template<class MemMap, class InputIterator, class MemRequest>
            static target::memory_region meet_request( const MemMap &memmap,
                                                      InputIterator omd_begin,
                                                      InputIterator omd_end,
                                                      const MemRequest &request )
            {
                utils::debug_assert( std::is_sorted( omd_begin, omd_end ),
                                    "The omd has to be sorted!" );
                
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
            
            /** \brief Calculate the available memory regions
             *         and apply a function to them.
             * \tparam MemMap The memory map type
             * \tparam InputIterator The omd iterator type
             * \tparam UnaryFunction The function object
             * \param[in] memmap The memory map
             * \param[in] omd_begin The begin of the omd
             * \param[in] omd_end The end of the omd
             * \param[in] function The function to be applied
             *            to every available memory region.
             *
             * \note The omd range has to be sorted in ascending
             *       order.
             */
            template<class MemMap, class InputIterator, class UnaryFunction>
            static void transform_avm( const MemMap &memmap,
                                      InputIterator omd_begin,
                                      InputIterator omd_end,
                                      UnaryFunction function )
            {
                utils::debug_assert( std::is_sorted( omd_begin, omd_end ),
                                    "The omd has to be sorted!" );
                
                for( auto desc_it = memmap.cbegin(); desc_it != memmap.cend(); ++desc_it )
                {
                    const auto &desc = *desc_it;
                    if( desc.type != memory_type::general_purpose )
                        continue;
                    
                    target::memory_region desc_region = { desc.virtual_start,
                        desc.number_of_pages * pagesize };
                    auto rest = desc_region;
                    
                    auto intersection = omd_begin;
                    while( (intersection = std::find_if( intersection, omd_end,
                                    std::bind( &target::memory_region::intersects_memory_region,
                                              rest,
                                              std::placeholders::_1 ) )) != omd_end )
                    {
                        if( rest.base() != intersection->base() )
                        {
                            target::memory_region av_region = { rest.base(),
                                intersection->base() - rest.base() };
                            function( av_region );
                        }
                        
                        if( intersection->top() > rest.top() )
                        {
                            rest = { rest.top(), 0 };
                            break;
                        }
                        
                        rest.size -= intersection->top() - rest.base();
                        rest.start = intersection->top();
                    }
                    
                    if( rest.base() != desc_region.top() )
                    {
                        target::memory_region av_region = { rest.base(),
                            desc_region.top() - rest.base() };
                        function( av_region );
                    }
                }
            }
            
            /** \brief Calculate the number of memory regions
             * \tparam MemMap The memory map type
             * \tparam InputIterator The omd iterator type
             * \param[in] memmap The memory map
             * \param[in] omd_begin The begin of the omd
             * \param[in] omd_end The end of the omd
             *
             * \returns The number of non-zero memory regions.
             *
             * \note The omd range has to be sorted in ascending
             *       order and be completely contained within
             *       the memory map otherwise the behaviour is
             *       undefined.
             */
            template<class MemMap, class InputIterator>
            static std::size_t
            number_of_avm_regions( const MemMap &memmap,
                                    InputIterator omd_begin,
                                    InputIterator omd_end )
            {
                std::size_t number_of_regions = 0;
                auto count = [&] ( const auto & ) {
                    number_of_regions++;
                };
                
                transform_avm( memmap, omd_begin, omd_end, count );
                return number_of_regions;
            }
            
            /** \brief Calculate the number of memory regions
             * \tparam MemMap The memory map type
             * \tparam InputIterator The omd iterator type
             * \param[in] memmap The memory map
             * \param[in] omd_begin The begin of the omd
             * \param[in] omd_end The end of the omd
             * \param[in] max_av_regions The maximum
             *            number of available memory regions.
             * \param[in] alloc The allocator to be used for the
             *            construction of the return value.
             *
             * \returns An array of \a std::memory_resource
             *          objects that cover the available memory
             *
             * \note \a alloc has to be able to allocate
             *       at least \a max_number_of_av_regions
             *       memory_resource objects otherwise
             *       the behaviour is undefined.
             * \note If max_av_regions is less than
             *       \a number_of_avm_regions() the behaviour
             *       is undefined.
             * \note The omd range has to be sorted in ascending
             *       order and be completely contained within
             *       the memory map otherwise the behaviour is
             *       undefined.
             */
            template<class MemMap, class InputIterator>
            static decltype(available_memory)
            enumerate_avm( const MemMap &memmap,
                          InputIterator omd_begin,
                          InputIterator omd_end,
                          decltype(available_memory)::allocator_type &&alloc )
            {
                /** \todo Perform some runtime size check." */
                
                auto num_av_regions = number_of_avm_regions( memmap, omd_begin, omd_end );
                
                auto av_regions = reinterpret_cast<target::memory_region *>(
                         UTOPIAOS_ALLOCA_WITH_ALIGN( num_av_regions * sizeof(target::memory_region),
                                                    alignof(target::memory_region) ) );
                target::memory_region *current = av_regions;
                
                auto assign = [&current] ( const auto &region ) {
                    *current++ = region;
                };
                
                transform_avm( memmap, omd_begin, omd_end, assign );
                
                using av_container = decltype(available_memory);
                using buffer_resource = typename av_container::value_type;
                auto buffer_constructor = [] ( decltype(available_memory)::value_type *buf,
                                              const target::memory_region &region ) {
                    new (buf) buffer_resource( target::uintptr_to_ptr<void>( region.base() ),
                                              region.size );
                };
                
                return av_container( &(av_regions[0]),
                                    current,
                                    std::move( alloc ),
                                    buffer_constructor );
            }
            
            struct address_of
            {
                template<class T>
                auto operator()( T &&t ) const
                { return std::addressof( std::forward<T>( t ) ); }
            };
            
            /** \brief Construct an \a unsynchronized_memory_manager from
             *         data computed by \a build_memory_manager()
             * \tparam MemMap The memory map type
             * \tparam InputIterator The omd iterator type
             * \tparam Resources The memory resource holding type
             * \param[in] mm The memory map
             * \param[in] omd_begin The begin of the omd
             * \param[in] omd_end The end of the omd
             * \param[in] max_av_regions The maximum
             *            number of available memory regions.
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
            : iresources( std::move( mr ) ),
            memmap( mm,
                   iresources[tag_index[boost::hana::type_c<memmap_memory_tag>]].get() ),
            omd( omd_begin, omd_end,
                iresources[tag_index[boost::hana::type_c<omd_memory_tag>]].get() ),
            available_memory( enumerate_avm( memmap, omd.cbegin(), omd.cend(),
                 iresources[tag_index[boost::hana::type_c<avm_memory_tag>]].get() ) ),
            avm_resource( boost::make_transform_iterator( available_memory.begin(),
                                                         address_of() ),
                         boost::make_transform_iterator( available_memory.end(),
                                                         address_of() ) ),
            subpage_resource( smallest_memory_chunk, pagesize,
                             pagesize, &avm_resource )
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
            
            /** \brief An unsynchronized_memory_manager is explicitly
             *         not copy-constructible!
             */
            unsynchronized_memory_manager( const unsynchronized_memory_manager & ) = delete;
            
            /** \brief An unsynchronized_memory_manager is explicitly
             *         move-constructible!
             */
            unsynchronized_memory_manager( unsynchronized_memory_manager && ) = default;
            
            /** \brief An unsynchronized_memory_manager is explicitly
             *         not copy-assignable!
             */
            unsynchronized_memory_manager &operator=( const unsynchronized_memory_manager & ) = delete;
            
            /** \brief An unsynchronized_memory_manager is explicitly
             *         move-assignable!
             */
            unsynchronized_memory_manager &operator=( unsynchronized_memory_manager && ) = default;
            
            /** \brief Returns a memory resource that can be used to
             *         allocate memory managed by the memory manager.
             * \returns A memory resource object that can be used to
             *          allocate chunks that fit into the kernel
             *          pagesize.
             */
            std::pmr::memory_resource *paged_resource( void )
            {
                return &subpage_resource;
            }
        };
    }
}

#endif

/** \} */
