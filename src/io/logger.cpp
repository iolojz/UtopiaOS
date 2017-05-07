/** \ingroup io
 * \{
 *
 * \file io/logger.cpp
 * \brief This file contains the implementation
 *        of some standard logging classes
 */

#if UTOPIAOS_HOSTED

#include "cstdarg"
#include "iostream"

#include "logger.hpp"

using namespace UtopiaOS;
using namespace io;

void cout_logger::log( unsigned number_of_strings, ... )
{
    va_list args;
    va_start( args, number_of_strings );
    
    try
    {
        while( number_of_strings != 0 )
        {
            std::cout << va_arg( args, const_stringref );
            number_of_strings--;
        }
    } catch( ... )
    {
        va_end( args );
        throw;
    }
    
    va_end( args );
}
#endif

/** \} */
