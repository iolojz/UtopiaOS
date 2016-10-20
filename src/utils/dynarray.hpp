/** \ingroup utils
 * \{
 *
 * \file utils/dynarray.hpp
 * \brief This file contains a simple dynarray
 *        implementation similar to the rejected
 *        c++14 proposal.
 */

#ifndef H_utils_dynarray
#define H_utils_dynarray

#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace UtopiaOS
{
    namespace utils
    {
        /** \class dynarray
         * \brief A dynarray is like the ordinary std::array
         *        but its length is fixed upon construction
         *        and not compile-time.
         * \tparam T The value type of the dynarray object
         * \tparam Allocator The allocator type
         *
         * For simplicity only copy-constructible allocators
         * are supported.
         */
        template<class T, class Allocator> class dynarray
        {
        public:
            using allocator_type = Allocator;
            using value_type = T;
            using size_type = std::size_t;
            using difference_type = std::make_signed<size_type>::type;
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
            
            template<class SomeAllocator>
            dynarray( std::initializer_list<value_type> init, SomeAllocator &&alloc )
            : dynarray( init.begin(), init.end(), std::forward<SomeAllocator>( alloc ) )
            {}
            
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
            
            /** \brief Construct a dynarray from another one
             *         by stealing its resources and keeping
             *         just the first \a length_to_preserve
             *         objects.
             * \param[inout] other The dynarray to steal resources from
             * \param[in] length_to_preserve The number of objects to
             *            keep.
             *
             * If the number of objects to keep is largeer than
             * the size of the original dynarray, a std::length_error
             * exception is raised and the original dynarray
             * is as untouched.
             */
            dynarray( dynarray &&other, size_type length_to_preserve )
            : allocator( std::move( other.allocator ) ),
            length( length_to_preserve ), buffer( other.buffer )
            {
                static_assert( std::is_nothrow_move_constructible<allocator_type>::value,
                              "allocator_type has to be nothrow move constructible" );
                
                if( length > other.length )
                {
                    other.allocator.~allocator_type();
                    new (&(other.allocator)) allocator_type( std::move( allocator ) );
                    throw std::length_error( "Attempt to construct a dynarray\
from another dynarray of shorter length than specified." );
                }
                
                other.length = 0;
                other.buffer = nullptr;
            }
            
            ~dynarray( void )
            {
                std::for_each( begin(), end(), [] ( reference r ) {
                    r.~value_type();
                } );
                
                // Allow for desctruction of a moved dynarray
                if( length != 0 )
                    allocator.deallocate( buffer, length );
            }
        };
    }
}

#endif

/** \} */
