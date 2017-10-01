/** \ingroup utils
 * \{
 *
 * \file utils/make_array.hpp
 * \brief This file defines convenience functions
 *        to ease the creation of \ std::array
 *        objects.
 */

#ifndef H_utils_make_array
#define H_utils_make_array

#include <type_traits>
#include <array>

namespace UtopiaOS
{
    namespace utils
    {
        /** \namespace UtopiaOS::utils::detail
         * \brief This namespace contains implementation
         *        details of the Utilities module.
         * \warning Do not use!
         */
        namespace detail
        {
            /** \brief Helper template for utils::make_array
             */
            template<class ForwardIterator, std::size_t N, class T>
            struct make_array_it
            {
                using decayed_type = typename std::decay<
                    typename std::iterator_traits<ForwardIterator>::value_type
                >::type;
                static constexpr bool use_cast = !std::is_same<decayed_type, T>::value;
                
                template<class... Args>
                static auto iterate( ForwardIterator begin, Args &&...args )
                -> typename std::enable_if<
                    !use_cast,
                    std::array<T, N + sizeof...(Args)>
                >::type
                {
                    ForwardIterator copy( begin );
                    return make_array_it<ForwardIterator, N-1, T>::iterate( ++begin,
                                                                        std::forward<Args>( args )...,
                                                                        *copy );
                }
                
                template<class... Args>
                static auto iterate( ForwardIterator begin, Args &&...args )
                -> typename std::enable_if<
                    use_cast,
                    std::array<T, N + sizeof...(Args)>
                >::type
                {
                    ForwardIterator copy( begin );
                    return make_array_it<ForwardIterator, N-1, T>::iterate( ++begin,
                                                                        std::forward<Args>( args )...,
                                                                        T( *copy ) );
                }
            };
            
            /** \brief Specialized helper template for utils::make_array */
            template<class ForwardIterator, class T>
            struct make_array_it<ForwardIterator, 0, T>
            {
                template<class... Args>
                static auto iterate( ForwardIterator, Args &&...args )
                {
                    return std::array<T, sizeof...(Args)>{ { std::forward<Args>( args )... } };
                }
            };
            
            /** \brief Helper template for utils::make_array
             */
            template<std::size_t N, class T>
            struct make_array_copy
            {
                template<class Init, class... Args>
                static auto copy( const Init &init, Args &&...args )
                {
                    return make_array_copy<N-1, T>::copy( init,
                                                         std::forward<Args>( args )...,
                                                         init );
                }
            };
            
            /** \brief Specialized helper template for utils::make_array */
            template<class T>
            struct make_array_copy<0, T>
            {
                template<class Init, class... Args>
                static auto copy( const Init &, Args &&...args )
                {
                    return std::array<T, sizeof...(Args)>{ std::forward<Args>( args )... };
                }
            };
        }
        
        /** \class make_array
         * \brief Facility for easy construction of \a std::array
         *        objects.
         * \tparam N The size of the to be constructed array
         * \tparam T The type of the objects the array should hold
         *           or void (by default) if the type should be
         *           inferred.
         */
        template<std::size_t N, class T = void>
        struct make_array
        {
            /** \brief Construct a \a std::array by copying values
             *         from a range.
             * \tparam ForwardIterator The iterator type (usually inferred)
             * \param[in] begin The iterator marking the beginning of
             *                  the range to be copied.
             * \returns A \a std::array holding the desired objects.
             * \warning \a begin must be incrementable sufficiently
             *          often (\a N times) otherwise the behaviour
             *          is undefined.
             */
            template<class ForwardIterator>
            static auto iterate( ForwardIterator begin )
            {
                using value_type = typename std::conditional<
                    std::is_void<T>::value,
                    typename std::iterator_traits<ForwardIterator>::value_type,
                    T
                >::type;
                
                return detail::make_array_it<ForwardIterator, N, value_type>::iterate( begin );
            }
            
            template<class Init>
            static auto copy( const Init &init )
            {
                using value_type = typename std::conditional<
                    std::is_void<T>::value,
                    typename std::decay<Init>::type,
                    T
                >::type;
                
                return detail::make_array_copy<N, value_type>::copy( init );
            }
        };
    }
}

#endif

/** \} */
