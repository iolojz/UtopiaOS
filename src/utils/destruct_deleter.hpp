#/** \ingroup utils
* \{
*
* \file utils/destruct_deleter.hpp
* \brief This file defines a deleter to be used
*        with \a std::unique_ptr that only
*        destructs the object, but does not
*        free the memory.
*/

#ifndef H_utils_destruct_deleter
#define H_utils_destruct_deleter

namespace UtopiaOS
{
    namespace utils
    {
        /** \class destruct_deleter
         * \brief A deleter class to be used with
         *        \a std::unique_ptr.
         * \tparam T The type of the
         *           to-be-destructed object
         */
        template<class T>
        struct destruct_deleter
        {
            /** \brief Call the destructor on an object
             * \param[in] t The object to be destroyed
             * \warning Passing a null pointer will
             *          result in undefined behaviour.
             */
            void operator()( T *t )
            {
                t->~T();
            }
        };
    }
}

#endif

/** \} */
