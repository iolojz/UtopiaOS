#/** \ingroup kernel
* \{
*
* \file kernel/distributed_resource.hpp
* \brief This file declares the
*        \a distributed_resource class,
*        which is a memory resource
*        with several upstream resources.
*/

#ifndef H_kernel_distributed_resource
#define H_kernel_distributed_resource

#include <initializer_list>
#include <memory_resource>
#include <stdexcept>
#include <memory>
#include <limits>

#include "utils/dynarray.hpp"
#include "target/memory.hpp"

namespace UtopiaOS
{
    namespace kernel
    {
        /** \class distributed_resource
         * \brief A memory resource class that
         *        forwards every allocation /
         *        deallocation request to one
         *        of its possibly several upstream
         *        memory resources.
         */
        class distributed_resource : public std::pmr::memory_resource
        {
        private:
            using resource_ptr_allocator =
                std::pmr::polymorphic_allocator<std::pmr::memory_resource *>;
            using resource_container = utils::dynarray<std::pmr::memory_resource *,
                resource_ptr_allocator
            >;
            
            resource_container resources; /**< The upstream resources */
            
            /** \brief Stores a range of memory resource
             *         pointers in a \a resource_container.
             * \tparam RandomAccessIterator The iterator type
             * \param[in] resource_begin The begin of the
             *                           resource range.
             * \param[in] resource_end The end of the
             *                         resource range.
             * \returns A \a resource_container object
             *          containing the pointers to the
             *          memory resource objects in the
             *          given range.
             * \throws std::bad_alloc if the range could
             *         not be constructed.
             * \note In order to construct the container,
             *       this function attempts to allocate
             *       some memory from the given resources.
             */
            template<class RandomAccessIterator>
            static auto
            store_resources( RandomAccessIterator resource_begin,
                            RandomAccessIterator resource_end )
            {
                for( auto it = resource_begin; it != resource_end; ++it )
                {
                    resource_ptr_allocator allocator( *it );
                    
                    try
                    {
                        return resource_container( resource_begin,
                                                  resource_end,
                                                  std::move( allocator ) );
                    } catch( const std::bad_alloc & )
                    {}
                }
                
                throw std::bad_alloc();
            }
            
            /** \brief Calculate the padding required
             *         for internal bookkeeping purposes
             * \param[in] bytes The number of bytes in
             *            the allocation request.
             * \param[in] alignment The required alignment
             *            in the allocation request.
             * \returns The padding that is necessary for
             *          internal bookkeeping purposes.
             */
            std::size_t required_padding( std::size_t bytes, std::size_t alignment )
            {
                std::size_t end_alignment_offset = ((alignment % alignof(std::size_t)) +
                                                    (bytes % alignof(std::size_t)));
                std::size_t padding;
                
                if( end_alignment_offset != 0 )
                {
                    padding = alignof(std::size_t) - end_alignment_offset;
                    if( std::numeric_limits<std::size_t>::max() - padding < bytes )
                        throw std::overflow_error( "" );
                } else
                    padding = 0;
                
                if( std::numeric_limits<std::size_t>::max() - sizeof(std::size_t) <
                   bytes + padding )
                    throw std::overflow_error( "" );
                
                return padding;
            }
            
            virtual void* do_allocate( std::size_t bytes, std::size_t alignment )
            {
                std::size_t padding;
                try
                {
                    padding = required_padding( bytes, alignment );
                } catch( const std::overflow_error & )
                {
                    throw std::bad_alloc();
                }
                
                std::size_t actual_size = bytes + padding + sizeof(std::size_t);
                
                for( std::size_t index = 0; index != resources.size(); index++ )
                {
                    void *memory = nullptr;
                    
                    try
                    {
                         memory = resources[index]->allocate( actual_size, alignment );
                    } catch( const std::bad_alloc & )
                    {}
                    
                    if( memory != nullptr )
                    {
                        static_assert( std::numeric_limits<std::uintptr_t>::max() >=
                                      std::numeric_limits<std::size_t>::max(),
                                      "This implementation cannot guarantee to work." );
                        
                        std::uintptr_t index_address = (target::ptr_to_uintptr( memory ) +
                                                        bytes + padding);
                        *target::uintptr_to_ptr<std::size_t>( index_address ) = index;
                        return memory;
                    }
                }
                
                throw std::bad_alloc();
            }
            
            virtual void do_deallocate( void* p, std::size_t bytes, std::size_t alignment )
            {
                std::size_t padding = required_padding( bytes, alignment );
                
                static_assert( std::numeric_limits<std::uintptr_t>::max() >=
                              std::numeric_limits<std::size_t>::max(),
                              "This implementation cannot guarantee to work." );
                
                std::uintptr_t index_address = (target::ptr_to_uintptr( p ) +
                                                bytes + padding);
                std::size_t index = *target::uintptr_to_ptr<std::size_t>( index_address );
                resources[index]->deallocate( p, bytes, alignment );
            }
            
            virtual bool do_is_equal( const std::pmr::memory_resource& other ) const
            {
                const distributed_resource *dr_other;
                try
                {
                    dr_other = dynamic_cast<const distributed_resource *>( std::addressof(other) );
                } catch( const std::bad_cast & )
                {
                    return false;
                }
                return (this == dr_other);
            }
        public:
            /** \brief Construct a \a distributed_resource
             *         object from a range of upstream
             *         memory resource objects
             * \tparam RandomAccessIterator The iterator type
             * \param[in] resource_begin The begin of the
             *                           resource range.
             * \param[in] resource_end The end of the
             *                         resource range.
             */
            template<class RandomAccessIterator>
            distributed_resource( RandomAccessIterator begin,
                                 RandomAccessIterator end )
            : resources( store_resources( begin, end ) )
            {}
        };
    }
}

#endif

/** \} */
