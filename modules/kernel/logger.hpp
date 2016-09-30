#ifndef H_kernel_logger
#define H_kernel_logger

#include "types.hpp"

#include <atomic>

namespace UtopiaOS
{
    namespace kernel
    {
        /** logger
         *
         * This class is an abstract base class used
         * for simple logging.
         * Any deriving class just needs to define
         * the logging of multiple (!) cstrings.
         * Implementing atomicity for the latter
         * is highly encouraged as is thread-safety.
         **/
        class logger
        {
        public:
            void log( un number_of_cstrings, ... ) = 0;
        };
        
        std::atomic<logger *> assertion_logger( nullptr );
        
        template<class ...STRINGS>
        void log( logger *l, STRINGS && ...strings )
        {
            if( l != nullptr )
                l->log( sizeof...(STRINGS), strings... );
        }
    }
}

#endif
