#/** \ingroup io
* \{
*
* \file io/logger.hpp
* \brief This file contains a simple logging system.
*/

#ifndef H_io_logger
#define H_io_logger

#include "string.hpp"

namespace UtopiaOS
{
    namespace io
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
            /** \brief Logs several ordered const_stringrefs */
            virtual void log( unsigned number_of_strings, ... ) = 0;
        };
        
#if UTOPIAOS_HOSTED
        /** \struct cout_logger
         * \brief This is a simple class that forwards
         *        all logging to std::cout in a non-synchronized
         *        way.
         */
        struct cout_logger : public logger
        {
            virtual void log( unsigned number_of_strings, ... ) override;
        };
#endif
        
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
                l->log( sizeof...(STRINGS), io::const_stringref(strings)... );
            }
        }
    }
}

#endif

/** \} */
