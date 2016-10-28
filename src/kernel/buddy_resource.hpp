#/** \ingroup kernel
* \{
*
* \file kernel/buddy_resource.hpp
* \brief This file declares the \a buddy_resource
*        class, that implements memory allocation
*        through the buddy method.
*/

#ifndef H_kernel_buddy_resource
#define H_kernel_buddy_resource

#include <initializer_list>
#include <memory_resource>
#include <memory>

#include <utils/dynarray.hpp>
#include <target/memory.hpp>

namespace UtopiaOS
{
    namespace kernel
    {
        template<std::size_t MinimumBlockSize>
        class buddy_resource : public std::pmr::memory_resource
        {
            static_assert( (MinimumBlockSize % 2) == 0,
                          "The minimum block size has to be a power of two." );
        private:
            using resource_ptr_allocator =
                std::pmr::polymorphic_allocator<std::pmr::memory_resource *>;
            using resource_container = utils::dynarray<std::pmr::memory_resource *,
            resource_ptr_allocator
            >;
            
            resource_container resources;
            
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
                
                throw std::runtime_error( "Cannot allocate enough memory for \
the buddy resource." );
            }
        public:
            buddy_resource( const
                           std::initializer_list<std::pmr::memory_resource *>
                           &upstream_resources )
            : resources( store_resources( upstream_resources.begin(),
                                         upstream_resources.end() ) )
            {}
            
            virtual void* do_allocate( std::size_t bytes, std::size_t alignment ) override;
            
            virtual void do_deallocate( void* p, std::size_t bytes, std::size_t alignment ) override;
            
            virtual bool do_is_equal( const std::pmr::memory_resource& other ) const override;
        };
    }
}

#endif

/** \} */
