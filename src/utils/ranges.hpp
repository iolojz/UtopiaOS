#/** \ingroup utils
* \{
*
* \file utils/ranges.hepp
* \brief This file defines template functions
*        that manipulate ranges.
*/

#ifndef H_utils_ranges
#define H_utils_ranges

#include <algorithm>

namespace UtopiaOS
{
    namespace utils
    {
        template<class InputIterator>
        class insertion_iterator
        {
        public:
            /** \name Iterator Traits
             * \{ */
            using iterator_category = typename std::iterator_traits<InputIterator>::iterator_category;
            using value_type = typename std::iterator_traits<InputIterator>::value_type;
            using reference = typename std::iterator_traits<InputIterator>::reference;
            using pointer = typename std::iterator_traits<InputIterator>::pointer;
            using difference_type = typename std::iterator_traits<InputIterator>::difference_type;
            /** \} */
        private:
            enum class region : signed
            {
                lower = -1,
                insertion = 0,
                upper = 1
            } current_region;
            
            InputIterator real_it;
            InputIterator end1;
            pointer inserted_element;
        public:
            insertion_iterator( InputIterator it, InputIterator e1, reference iel )
            : current_region( (it < e1) ? region::lower : region::upper ),
            real_it( it ), end1( e1 ), inserted_element( &iel ) {}
            
            insertion_iterator &operator++( void )
            {
                switch( current_region )
                {
                    case region::lower:
                        if( ++real_it == end1 )
                            current_region = region::insertion;
                        break;
                    case region::insertion:
                        current_region = region::upper;
                        break;
                    default: //case region::upper:
                        ++real_it;
                        break;
                }
                
                return *this;
            }
            
            insertion_iterator operator++( int )
            { insertion_iterator cp( *this ); ++(*this); return cp; }
            
            insertion_iterator &operator--( void )
            {
                switch( current_region )
                {
                    case region::lower:
                        --real_it;
                        break;
                    case region::insertion:
                        current_region = region::lower;
                        --real_it;
                        break;
                    default: //case region::upper:
                        if( real_it == end1 )
                            current_region = region::insertion;
                        break;
                }
                
                return *this;
            }
            insertion_iterator operator--( int )
            { insertion_iterator cp( *this ); --(*this); return cp; }
            
            reference operator*( void ) const
            {
                if( current_region == region::insertion )
                    return *inserted_element;
                return *real_it;
            }
            pointer operator->( void ) const
            {
                if( current_region == region::insertion )
                    return inserted_element;
                return &(*real_it);
            }
            
            bool operator==( const insertion_iterator &it ) const
            { return (current_region == it.current_region && real_it == it.real_it); }
            bool operator!=( const insertion_iterator &it ) const
            { return !(*this == it); }
            
            bool operator<( const insertion_iterator &it ) const
            {
                switch( current_region )
                {
                    case region::lower:
                        if( it.current_region != region::lower )
                            return true;
                        return real_it < it.real_it;
                    case region::insertion:
                        return current_region < it.current_region;
                    default: //case region::upper:
                        if( it.current_region != region::upper )
                            return false;
                        return real_it < it.real_it;
                }
            }
            bool operator<=( const insertion_iterator &it ) const
            { return (*this == it || *this < it); }
            bool operator>=( const insertion_iterator &it ) const
            { return !(*this < it); }
            bool operator>( const insertion_iterator &it ) const
            { return !(*this <= it); }
            
            insertion_iterator &operator+=( difference_type n )
            {
                static_assert( std::is_same<iterator_category,
                              std::random_access_iterator_tag>::value,
                              "template type parameter InputIterator is not a \
random access iterator" );
                
                if( n < 0 )
                    return (*this -= -n);
                
                switch( current_region )
                {
                    case region::lower:
                    {
                        difference_type diff1 = end1 - real_it;
                        if( n < diff1 )
                            real_it += n;
                        else if( n == diff1 )
                        {
                            current_region = region::insertion;
                            real_it = end1;
                        } else
                        {
                            current_region = region::upper;
                            real_it += n-1;
                        }
                        break;
                    }
                    case region::insertion:
                        if( n != 0 )
                        {
                            current_region = region::upper;
                            real_it += n-1;
                        }
                        break;
                    default: //case region::upper:
                        real_it += n;
                }
                
                return *this;
            }
            insertion_iterator operator+( difference_type n ) const
            { insertion_iterator cp( *this ); cp += n; return cp; }
            
            insertion_iterator &operator-=( difference_type n )
            {
                static_assert( std::is_same<iterator_category,
                              std::random_access_iterator_tag>::value,
                              "template type parameter InputIterator is not a \
random access iterator" );
                
                if( n < 0 )
                    return (*this += -n);
                
                switch( current_region )
                {
                    case region::lower:
                        real_it -= n;
                        break;
                    case region::insertion:
                        if( n != 0 )
                        {
                            current_region = region::lower;
                            real_it -= n;
                        }
                        break;
                    default: //case region::upper:
                    {
                        difference_type diff1 = real_it - end1;
                        if( n <= diff1 )
                            real_it -= n;
                        else if( n == diff1 + 1 )
                        {
                            current_region = region::insertion;
                            real_it = end1;
                        } else
                        {
                            current_region = region::lower;
                            real_it -= n-1;
                        }
                    }
                }
                
                return *this;
            }
            insertion_iterator operator-( difference_type n ) const
            { insertion_iterator cp( *this ); cp -= n; return cp; }
            
            difference_type operator-( insertion_iterator it ) const
            {
                static_assert( std::is_same<iterator_category,
                              std::random_access_iterator_tag>::value,
                              "template type parameter InputIterator is not a \
random access iterator" );
                
                switch( current_region )
                {
                    case region::lower:
                        if( it.current_region == region::lower )
                            return real_it - it.real_it;
                        return (real_it - it.real_it) - 1;
                    case region::insertion:
                        if( it.current_region != region::upper )
                            return end1 - it.real_it;
                        return (end1 - it.real_it) - 1;
                    default: //case region::upper:
                        if( it.current_region == region::upper )
                            return real_it - it.real_it;
                        return (real_it - it.real_it) + 1;
                        
                }
            }
            
            reference operator[]( difference_type n ) const
            {
                static_assert( std::is_same<iterator_category,
                              std::random_access_iterator_tag>::value,
                              "template type parameter InputIterator is not a \
random access iterator" );
                
                return *(*this + n);
            }
        };
        
        template<class InputIterator, class T>
        auto range_by_inserting_reference( InputIterator begin,
                                        InputIterator end1,
                                        T &&value,
                                        InputIterator end2 )
        {
            using iterator = insertion_iterator<InputIterator>;
            return std::make_pair( iterator( begin, end1, std::forward<T>( value ) ),
                                  iterator( end2, end1, std::forward<T>( value ) ) );
        }
        
        template<class InputIterator, class T>
        auto sorted_range_insert_reference( InputIterator begin,
                                           InputIterator end,
                                           T &&value )
        {
            using value_type = typename std::iterator_traits<InputIterator>::value_type;
            auto larger = std::find_if( begin, end, [&value] ( const value_type &compare ) {
                return !(compare < value_type( value ));
            } );
            
            return range_by_inserting_reference( begin, larger, std::forward<T>( value ), end );
        }
    }
}

#endif

/** \} */
