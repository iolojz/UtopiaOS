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
#include <memory>

namespace UtopiaOS
{
    namespace utils
    {
        /** \brief Given a boost range object and a location,
         *         create a boost range object with one given
         *         element inserted at that location.
         * \tparam Range The boost range type
         * \tparam RangeIterator The boost range iterator type
         * \tparam T The reference type
         * \param[in] range The boost range object
         * \param[in] location The location at which to insert
         *                     the reference
         * \param[in] ref The reference to be inserted
         *
         * \returns A range object with the reference inserted
         *          at the given position.
         */
        template<class Range, class RangeIterator, class T>
        auto range_by_inserting_reference( const Range &range,
                                          RangeIterator location,
                                          T &&ref )
        {
            auto lower_range = boost::make_iterator_range( boost::begin( range ),
                                                           location );
            auto ref_range = boost::make_iterator_range( std::addressof( ref ),
                                                        std::addressof( ref ) + 1 );
            auto upper_range = boost::make_iterator_range( location,
                                                           boost::end( range ) );
            
            return  boost::join( boost::join( lower_range, ref_range ), upper_range );
        }
        
        /** \brief Given a sorted boost range object, create a
         *         boost range object with one more element that
         *         is still sorted.
         * \tparam Range The boost range type
         * \tparam T The reference type
         * \param[in] range The boost range object
         * \param[in] ref The reference to be inserted
         *
         * \returns A range object with one more element that is
         *         still sorted.
         *
         * \note The input range has to be sorted, otherwise the
         *       behaviour is undefined.
         */
        template<class Range, class T>
        auto sorted_range_insert_reference( const Range &range, T &&ref )
        {
            debug_assert( std::is_sorted( boost::begin( range ), boost::end( range ) ),
                         "The input range has to be sorted." );
            
            auto larger = std::find_if( boost::begin( range ),
                                       boost::end( range ),
                                       [&ref] ( const auto &compare ) {
                return !(compare < ref);
            } );
            
            return range_by_inserting_reference( range, larger, std::forward<T>( ref ) );
        }
    }
}

#endif

/** \} */
