#ifndef H_kernel_dynarray
#define H_kernel_dynarray

#include "types.hpp"

#include <algorithm>

namespace UtopiaOS
{
    namespace kernel
    {
        template<typename T> class dynarray
        {
        public:
            using value_type = T;
            using size_type = un;
            using difference_type = sn;
            using reference = T &;
            using const_reference = const T &;
            using pointer = T *;
            using const_pointer = const T *;
            using iterator = T *;
            using const_iterator = const T *;
            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::const_reverse_iterator<iterator>;
            
        private:
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
            dynarray( dynarray && ) = default;
            
            dynarray &operator=( const dynarray & ) = delete;
            dynarray &operator=( dynarray && ) = delete;
            
            template<class RandomAccessIterator, class Allocator>
            dynarray( RandomAccessIterator first, RandomAccessIterator last,
                     const Allocator &allocator )
            : length( last - first ), buffer( allocator( length ) )
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
                    while( current != first )
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
                std::foreach( begin(), end(), [] ( reference r ) {
                    r.~value_type();
                } );
                             
                allocator.deallocate( buffer, length );
            }
        };
    }
}

#endif
