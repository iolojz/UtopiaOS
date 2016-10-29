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

#include <memory_resource>

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
            std::pmr::memory_resource *upstream;
        public:
            buddy_resource( std::pmr::memory_resource *upstream_resource )
            : upstream( upstream_resource )
            {}
            
            virtual void* do_allocate( std::size_t bytes, std::size_t alignment );
            
            virtual void do_deallocate( void* p, std::size_t bytes, std::size_t alignment );
            
            virtual bool do_is_equal( const std::pmr::memory_resource& other ) const;
        };
    }
}

#endif

/** \} */
