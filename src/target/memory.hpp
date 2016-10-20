/** \ingroup target
 * \{
 *
 * \file target/memory.hpp
 * \brief This file contains basic memory-related
 *        complete types that are target-agnostic
 */

#ifndef H_target_memory
#define H_target_memory

#include <limits>
#include <cstdint>
#include <cstddef>

#include <utils/debug.hpp>

namespace UtopiaOS
{
    namespace target
    {
        /** \brief pointer to std::uintptr_t conversion function.
         * \tparam T The type of object the pointer points to (usually inferred).
         * \param[in] ptr The pointer that is to be converted
         * \return A std::uintptr_t carrying the value of \a ptr
         */
        template<class T>
        std::uintptr_t ptr_to_uintptr( T *ptr )
        { return reinterpret_cast<std::uintptr_t>( ptr ); }
        
        /** \brief std::uintptr_t to pointer conversion function.
         * \tparam T The type of object the pointer should point to.
         * \param[in] u The std::uintptr_t that is to be converted
         * \return A T * pointing to the location specified by \a u
         */
        template<class T>
        T *uintptr_to_ptr( std::uintptr_t u )
        { return reinterpret_cast<T*>( u ); }
        
        /** \struct memory_region
         * \brief This struct represents a memory region
         *        in some address map, not necessarily the
         *        current one
         */
        struct memory_region
        {
            /** \brief The start of the memory region */
            std::uintptr_t start;
            /** \brief The size of the memory region */
            std::size_t size;
            
            std::uintptr_t base( void ) const
            { return start; }
            std::uintptr_t top( void ) const
            { return start + size; }
            
            void *base_ptr( void ) const
            { return uintptr_to_ptr<void>( start ); }
            
            /** \brief Checks whether the memory region and
             *         another given memory region intersect.
             * \param[in] region The other memory region
             * \returns \a true if they intersect, false otherwise
             */
            bool intersects_memory_region( const memory_region &region ) const
            {
                /** \todo static_assert interger requirements */
                
                if( region.base() < base() )
                    return (region.top() > base());
                if( region.base() < top() )
                    return true;
                
                return false;
            }
            
            bool operator<( const memory_region &other ) const
            { return start < other.start; }
        };
        
        /** \struct memory_request
         * \brief Encapsulates a memory requirement
         * \tparam ALIGN Specifies the alignment
         */
        template<std::size_t ALIGN>
        struct memory_request
        {
            std::size_t size;
            static constexpr std::size_t alignment = ALIGN;
        };
        
        /** \brief Aligns a pointer to a given alignment.
         * \tparam ALIGN The alignment to meet
         * \param[in] ptr The pointer to align
         * \returns An aligned pointer or 0 if the pointer
         *          could not be aligned.
         *
         * The aligned pointer is guaranteed to point to
         * an address not smaller than the original pointer.
         * It is guaranteed that there is no other aligned
         * address in the range [\a ptr, \a return-value].
         * If the pointer cannot be aligned, i.e it points
         * to an address so large that the next aligned
         * address would not fit into the data type,
         * 0 is returned.
         *
         * \warning If \a alignment is not a power of two,
         *          the behaviour is undefined!
         */
        template<std::size_t ALIGN>
        constexpr std::uintptr_t align( std::uintptr_t ptr )
        {
            static_assert( std::numeric_limits<std::uintptr_t>::max() >=
                          std::numeric_limits<std::size_t>::max(),
                          "This implementation cannot guarantee to work." );
            utils::debug_assert( (ALIGN % 2) == 0, "alignment has to be a power of two" );
            
            std::size_t mask = ALIGN - 1; // ...0000011111...
            std::size_t diff = (ptr & mask);
            
            if( diff == 0 )
                return ptr;
            
            return (ptr + (ALIGN - diff));
        }
    }
}

#endif

/** \} */
