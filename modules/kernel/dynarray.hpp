/** \ingroup kernel
 * \{
 *
 * \file kernel/dynarray.hpp
 * \brief This file contains a simple dynarray
 *        implementation similar to the rejected
 *        c++14 proposal.
 */

#ifndef H_kernel_dynarray
#define H_kernel_dynarray

#include <common/types.hpp>

#include <algorithm>
#include <iterator>

namespace UtopiaOS
{
    namespace kernel
    {
        /** \class dynarray
         * \brief A dynarray is like the ordinary std::array
         *        but its length is fixed upon construction
         *        and not compile-time.
         * \tparam T The value type of the dynarray object
         * \tparam Allocator the allocator type
         *
         * \warning 
         */
        template<class T, class Allocator> class dynarray
        {
        public:
            using allocator_type = Allocator;
            using value_type = T;
            using size_type = common::un;
            using difference_type = common::sn;
            using reference = T &;
            using const_reference = const T &;
            using pointer = T *;
            using const_pointer = const T *;
            using iterator = T *;
            using const_iterator = const T *;
            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;
            
        private:
            allocator_type allocator;
            size_type length;
            pointer buffer;
        public:
            size_type size( void ) const { return length; }
            
            iterator begin( void ) { return data(); }
            const_iterator begin( void ) const { return data(); }
            const_iterator cbegin( void ) const { return data(); }
            iterator end( void ) { return data() + size(); }
            const_iterator end( void ) const { return data() + size(); }
            const_iterator cend( void ) const { return data() + size(); }
            
            reverse_iterator rbegin( void ) { return { end() }; }
            const_reverse_iterator rbegin( void ) const { return { end() }; }
            const_reverse_iterator crbegin( void ) const { return { cend() }; }
            reverse_iterator rend( void ) { return { begin() }; }
            const_reverse_iterator rend( void ) const { return { begin() }; }
            const_reverse_iterator crend( void ) const { return { cbegin() }; }
            
            reference front() { return *begin(); }
            const_reference front() const { return *begin(); }
            reference back() { return *(begin()+size()-1); }
            const_reference back() const { return *(begin()+size()-1); }
            pointer data() { return buffer; }
            const_pointer data() const { return buffer; }
            reference operator[]( size_type i ) { return data()[i]; }
            const_reference operator[]( size_type i ) const { return data()[i]; }
            
            dynarray( const dynarray & ) = default;
            
            dynarray &operator=( const dynarray & ) = delete;
            dynarray &operator=( dynarray && ) = delete;
            
            dynarray( std::initializer_list<value_type> init, allocator_type &&alloc )
            : dynarray( init.begin(), init.end(), alloc ) {}
            
            template<class RandomAccessIterator>
            dynarray( RandomAccessIterator first, RandomAccessIterator last,
                     allocator_type &&alloc )
            : dynarray( first, last, alloc ) {}
            
            template<class RandomAccessIterator>
            dynarray( RandomAccessIterator first, RandomAccessIterator last,
                     const allocator_type &alloc )
            : allocator( alloc ), length( last - first ),
            buffer( allocator.allocate( length ) )
            {
                pointer current = buffer;
                try
                {
                    while( first != last )
                    {
                        new (current) value_type( *first );
                        ++first;
                    }
                } catch( ... )
                {
                    while( current != buffer )
                    {
                        --current;
                        current->~value_type();
                    }
                    
                    allocator.deallocate( buffer, length );
                    throw;
                }
            }
            
            ~dynarray( void )
            {
                std::for_each( begin(), end(), [] ( reference r ) {
                    r.~value_type();
                } );
                             
                allocator.deallocate( buffer, length );
            }
        };
    }
}

#endif

/** \} */
