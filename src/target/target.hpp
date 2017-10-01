/** \defgroup target Target
 * \brief This module is header-only and describes
 *        the target-dependent aspects of the OS.
 * \{
 *
 * \file target/config.hpp
 * \brief This file provides the configurable options
 *        as well as definitions for target-dependent macros.
 */

#ifndef H_target_config
#define H_target_config

#include <target/config.hpp>

/** \def UTOPIAOS_ENABLE_DEBUG_ASSERTS
 * \brief Specifies whether assertions meant for
 *        debugging purposes should be checked.
 */
#define UTOPIAOS_ENABLE_DEBUG_ASSERTS (1)

/** \def UTOPIAOS_KERNEL_PAGESIZE
 * \brief Specifies the pagesize used by the kernel.
 */
#define UTOPIAOS_KERNEL_PAGESIZE (std::size_t(1) << 12)

/** \def UTOPIAOS_TRAP()
 * \brief Causes an immediate halt of the current
 *        thread of execution.
 */
#define UTOPIAOS_TRAP() __builtin_trap()

/** \def UTOPIAOS_ALLOCA_WITH_ALIGN_HEADER
 * \brief If this macro is defined it contains the header
 *        that needs to be included in order to use
 *        \a UTOPIAOS_ALLOCA_WITH_ALIGN
 */
#define UTOPIAOS_ALLOCA_WITH_ALIGN_HEADER <climits>

/** \def UTOPIAOS_ALLOCA_WITH_ALIGN( size, alignment )
 * \brief Requests a memory block of specified size and alignment
 *        that is automatically freed upon the end of the current
 *        block.
 * \param[in] size The size of the requested memory block
 * \param[in] alignment The compiletime-constant alignment
 *            of the requested memory block (in bytes)
 * \return A void * pointing to the newly allocated memory block
 *
 * \warning \a alignment has to be a power of two!
 */
#define UTOPIAOS_ALLOCA_WITH_ALIGN( size, alignment ) \
__builtin_alloca_with_align( (size), ((CHAR_BIT)*(alignment)) )

/** \name UEFI compatible types
 * \todo These should be automatically generated
 *       by Cmake or a configure script.
 * \{ */
#define UTOPIAOS_UEFI_UN unsigned long
#define UTOPIAOS_UEFI_UINT32 unsigned int
#define UTOPIAOS_UEFI_UINT64 unsigned long
/** \} */

#endif /* H_common_target_headers */

/** \} */
