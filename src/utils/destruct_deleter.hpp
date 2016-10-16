#/** \ingroup utils
* \{
*
* \file utils/destruct_deleter.hpp
* \brief This file defines a deleter to be used
*        with std::unique_ptr that only
*        destructs the object, but does not
*        free the memory.
*/

#ifndef H_utils_destruct_deleter
#define H_utils_destruct_deleter

namespace UtopiaOS
{
    namespace utils
    {
        template<class T>
        struct destruct_deleter
        {
            void operator()( T *t )
            {
                t->~T();
            }
        };
    }
}

#endif

/** \} */
