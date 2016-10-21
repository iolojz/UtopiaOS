/** \ingroup utils
 * \{
 *
 * \file utils/constructor.hpp
 * \brief This file defines the default
 *        constructor used by \a dynarray.
 */

#ifndef H_utils_constructor
#define H_utils_constructor

namespace UtopiaOS
{
    namespace utils
    {
        /** \class new_constructor
         * \brief A simple function object that only
         *        uses the new expression to construct
         *        objects.
         * \tparam T The type of the objects to be
         *           constructed
         */
        template<class T>
        struct new_constructor
        {
            /** \brief Simply use the new expression with
             *         forwarded arguments.
             * \tparam Args The argument types
             * \param[inout] location The location at which
             *               the object should be constructed
             * \param[in] args The arguments to be forwarded
             *            to the objects constructor.
             */
            template<class... Args>
            void operator()( T *location, Args &&...args )
            {
                new (location) T( std::forward<Args>( args )... );
            }
        };
    }
}

#endif

/** \} */
