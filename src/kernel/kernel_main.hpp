/** \defgroup kernel Kernel
 * \brief This module defines the actual kernel!
 * \{
 *
 * \file kernel/kernel_main.hpp
 * \brief This file contains the declaration of the
 *        kernel_main() function used to boot the kernel.
 */

#ifndef H_kernel_kernel_main
#define H_kernel_kernel_main

namespace UtopiaOS
{
    class environment;
    
    /** \namespace UtopiaOS::kernel
     * \brief Everything in the module Kernel goes
     *        into this namespace.
     */
    namespace kernel
    {
        /** \brief The boot function of the kernel.
         * \param[in] env The data passed on from the bootloader
         *
         * \warning This function never returns!
         * \note This function prototype needs to be API/ABI-stable
         *       and hence may not be changed!
         */
        [[noreturn]] void kernel_main( const environment *env );
    }
}

#endif

/** \} */
