#/** \ingroup utils
* \{
*
* \file utils/ranges.hpp
* \brief This file defines template functions
*        that manipulate iterator ranges.
*/

#ifndef H_utils_ranges
#define H_utils_ranges

#include <algorithm>
#include <type_traits>

namespace UtopiaOS
{
    namespace utils
    {
        namespace detail
        {
            /** \enum region
             * \brief Used by \a insertion_iterator to specify
             *        the current iterator location.
             */
            enum class region : signed
            {
                lower = -1,
                insertion = 0,
                upper = 1
            };
            
            /** \class insertion_iterator
             * \brief An iterator class wrapping a given
             *        iterator range and an arbitrarily
             *        inserted reference in a new
             *        iterator class.
             * \tparam InputIterator The wrapped iterator class.
             *                       Has to be an input iterator.
             */
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
                
                static constexpr bool isInput = std::is_base_of<std::input_iterator_tag,
                iterator_category
                >::value;
                
                static_assert( isInput, "Only input iterators are supported" );
            private:
                region current_region; /**< Represents the current location of the iterator. */
                
                InputIterator real_it; /**< The real iterator object */
                InputIterator end1; /**< The position of the inserted element */
                pointer inserted_element; /**< A pointer to the inserted element */
            public:
                /** \brief Default construction of the iterator
                 * \note Dereferencing a default-constructed iterator
                 *       is undefined behaviour.
                 */
                insertion_iterator( void ) {}
                
                /** \brief Construct an insertion iterator
                 * \param[in] it An interator representing to the current position
                 * \param[in] e1 An iterator representing the location of the
                 *               inserted element.
                 * \param[in] iel A reference to the inserted element
                 */
                insertion_iterator( InputIterator e1, reference iel,
                                   region r, InputIterator current )
                : current_region( r ), real_it( current ),
                end1( e1 ), inserted_element( &iel ) {}
                
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
                    return *(*this + n);
                }
            };
        }
        
        /** \brief Construct an iterator range by inserting
         *         an extra reference element into a range
         *         at a given position.
         * \tparam InputIterator The iterator type
         * \tparam T The reference type
         * \param[in] begin The begin of the iterator range
         * \param[in] location The location at which the reference
         *                 should be inserted.
         * \param[in] ref  The reference to be inserted
         * \param[in] end The end of the iterator range
         */
        template<class InputIterator, class T>
        auto range_by_inserting_reference( InputIterator begin,
                                        InputIterator location,
                                        T &&ref,
                                        InputIterator end )
        {
            using iterator = detail::insertion_iterator<InputIterator>;
            
            iterator new_begin;
            
            if( begin == end1 )
                new_begin = iterator( location, std::forward<T>( ref ),
                                          detail::region::insertion, begin );
            else
                new_begin = iterator( location, std::forward<T>( ref ),
                                     detail::region::lower, begin );
            
            auto new_end = iterator( location, std::forward<T>( ref ),
                                    detail::region::upper, end );
            
            return std::make_pair( new_begin, new_end );
        }
        
        /** \brief Construct an iterator range by inserting
         *         an extra reference element into a
         *         sorted range.
         * \tparam InputIterator The iterator type
         * \tparam T The reference type
         * \param[in] begin The begin of the iterator range
         * \param[in] end The end of the iterator range
         * \param[in] ref The reference to be inserted
         *
         * The input range has to be sorted.
         */
        template<class InputIterator, class T>
        auto sorted_range_insert_reference( InputIterator begin,
                                           InputIterator end,
                                           T &&ref )
        {
            debug_assert( std::is_sorted( begin, end ),
                         "The input range has to be sorted." );
            
            using value_type = typename std::iterator_traits<InputIterator>::value_type;
            auto larger = std::find_if( begin, end, [&ref] ( const value_type &compare ) {
                return !(compare < value_type( ref ));
            } );
            
            return range_by_inserting_reference( begin, larger, std::forward<T>( ref ), end );
        }
    }
}

#endif

/** \} */
