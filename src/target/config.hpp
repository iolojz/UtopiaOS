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

/** \def UTOPIAOS_ENABLE_DEBUG_ASSERTS
 * \brief Specifies whether assertions meant for
 *        debugging purposes should be checked.
 */
#define UTOPIAOS_ENABLE_DEBUG_ASSERTS (1)

/** \def UTOPIAOS_KERNEL_PAGESIZE
 * \brief Specifies the pagesize used by the kernel.
 */
#define UTOPIAOS_KERNEL_PAGESIZE (1 << 12)

/** \def UTOPIAOS_TRAP()
 * \brief Causes an immediate halt of the current
 *        thread of execution.
 */
#define UTOPIAOS_TRAP() __builtin_trap()

/** \def UTOPIAOS_ALLOCA_WITH_ALIGN( size, alignment )
 * \brief Requests a memory block of specified size and alignment
 *        that is automatically freed upon the end of the current
 *        block.
 * \param[in] size The size of the requested memory block
 * \param[in] alignment The compiletime-constant alignment
 *            of the requested memory block
 * \return A void * pointing to the newly allocated memory block
 *
 * \warning \a alignment has to be a power of two!
 */
#define UTOPIAOS_ALLOCA_WITH_ALIGN( size, alignment ) __builtin_alloca_with_align( (size), (alignment) )

#define UTOPIAOS_UEFI_UN unsigned long
#define UTOPIAOS_UEFI_UINT32 unsigned int
#define UTOPIAOS_UEFI_UINT64 unsigned long

#define UTOPIAOS_HOSTED 1

#endif /* H_common_target_headers */

/** \} */
