#/** \ingroup kernel
* \{
*
* \file kernel/logger.hpp
* \brief This file contains a simple logging system.
*/

#ifndef H_kernel_logger
#define H_kernel_logger

#include <common/types.hpp>

#include <atomic>

#include "dynarray.hpp"

namespace UtopiaOS
{
    namespace kernel
    {
        /** \class logger
         * \brief This class is an abstract base class used
         * for simple logging.
         *
         * The logging protocol is kept this simple so that
         * it can be used throughout the OS simply by
         * changing the backends appropriately.
         * Any deriving class just needs to define
         * the logging of multiple (!) const_stringrefs.
         * Implementing atomicity for the latter
         * is highly encouraged as is thread-safety.
         **/
        class logger
        {
        public:
            /** \brief Logs several ordered strings */
            virtual void log( common::un number_of_strings, ... ) = 0;
        };
        
        /** \brief The global assertion logger object,
         *         initialized to nullptr!
         */
        std::atomic<logger *> assertion_logger( nullptr );
        
        /** \brief Print strings to log.
         * \tparam STRINGS Needs to be convertible to const_stringrefs
         * \param[in] l the desired logger
         * \param[in] strings The strings to be printed (in order)
         *
         * A nullptr valued logger is silently ignored.
         */
        template<class ...STRINGS>
        void log( logger *l, STRINGS && ...strings )
        {
            if( l != nullptr )
            {
                l->log( sizeof...(STRINGS), common::const_stringref(strings)... );
            }
        }
    }
}

#endif

/** \} */
