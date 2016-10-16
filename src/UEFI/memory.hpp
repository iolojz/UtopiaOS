/** \ingroup UEFI
 * \{
 *
 * \file UEFI/memory.hpp
 * \brief This file contains basic memory-related
 *        types and constants modelled to be UEFI
 *        compatible.
 */

#ifndef H_UEFI_memory
#define H_UEFI_memory

#include <target/UEFItypes.hpp>
#include <target/memory.hpp>

#include <iterator>

namespace UtopiaOS
{
    /** \namespace UtopiaOS::UEFI
     * \brief This namespace contains types and constants
     *        geared towards a UEFI compatible interface
     */
    namespace UEFI
    {
        /** \typedef un
         * \brief UEFI native unsigned integer type
         */
        using un = target::UEFI::un;
        
        /** \typedef uint32
         * \brief UEFI unsigned integer type
         *        with exactly 32 usable bits.
         */
        using uint32 = target::UEFI::uint32;
        
        /** \typedef uint64
         * \brief Signed integer type
         *        with exactly 32 usable bits.
         */
        using uint64 = target::UEFI::uint64;
        
        /** \enum memory_type
         * \brief Equivalent to the UEFI type EFI_MEMORY_TYPE */
        enum class memory_type : uint32
        {
            EfiReservedMemoryType = 0,
            EfiLoaderCode,
            EfiLoaderData,
            EfiBootServicesCode,
            EfiBootServicesData,
            EfiRuntimeServicesCode,
            EfiRuntimeServicesData,
            EfiConventionalMemory,
            EfiUnusableMemory,
            EfiACPIReclaimMemory,
            EfiACPIMemoryNVS,
            EfiMemoryMappedIO,
            EfiMemoryMappedIOPortSpace,
            EfiPalCode,
            EfiMaxMemoryType
        };
        
        /** \brief A constant holding the UEFI pagesize (4KiB) */
        static constexpr un pagesize = (un(1) << 12);
        
        /** \typedef physical_address
         * \brief Equivalent to the UEFI type EFI_PHYSICAL_ADDRESS
         */
        using physical_address = uint64;
        /** \typedef physical_address
         * \brief Equivalent to the UEFI type EFI_VIRTUAL_ADDRESS
         */
        using virtual_address = uint64;
        
        /** \name Memory Attribute Definitions 
         * \brief Constants equivalent to the UEFI memory attribute definitions
         * \{ */
        static constexpr uint64 EFI_MEMORY_UC      = (uint64(1) <<  0);
        static constexpr uint64 EFI_MEMORY_WC      = (uint64(1) <<  1);
        static constexpr uint64 EFI_MEMORY_WT      = (uint64(1) <<  2);
        static constexpr uint64 EFI_MEMORY_WB      = (uint64(1) <<  3);
        static constexpr uint64 EFI_MEMORY_UCE     = (uint64(1) <<  4);
        static constexpr uint64 EFI_MEMORY_WP      = (uint64(1) << 12);
        static constexpr uint64 EFI_MEMORY_RP      = (uint64(1) << 13);
        static constexpr uint64 EFI_MEMORY_XP      = (uint64(1) << 14);
        static constexpr uint64 EFI_MEMORY_RUNTIME = (uint64(1) << 63);
        /** \} */
        
        /** \struct memory_descriptor_v1
         * \brief Equivalent to a UEFI memory descriptor
         *        when in the UEFI firmware
         *        EFI_MEMORY_DESCRIPTOR_VERSION is set to 1
         */
        struct memory_descriptor_v1
        {
            memory_type type;
            physical_address physical_start; /**< 4KiB aligned! */
            virtual_address virtual_start; /**< 4KiB aligned! */
            uint64 number_of_pages; /**< number of 4KiB pages */
            uint64 attribute;
        };
        
        class memory_map;
        
        /** \class const_memory_map_iterator_v1
         * \brief An iterator class that can be used to
         *        traverse the elements of a UEFI memory map.
         *
         * This iterator will always extract the UEFI memory
         * descriptors as of EFI_MEMORY_DESCRIPTOR_VERSION 1
         * no matter what higher version the UEFI memory map
         * might support.
         */
        class const_memory_map_iterator_v1
        {
        public:
            friend class UEFI::memory_map;
            
            /** \name Iterator Traits
             * \{ */
            using iterator_category = std::random_access_iterator_tag;
            using value_type = const memory_descriptor_v1;
            using reference = const memory_descriptor_v1 &;
            using pointer = const memory_descriptor_v1 *;
            using difference_type = std::make_signed<un>::type;
            /** \} */
            
            pointer descriptor; /**< The current memory descriptor */
            un descriptor_size; /**< The real size of the memory descriptor */
        private:
            /** \brief Construct an iterator from data supplied by a UEFI memory map */
            const_memory_map_iterator_v1( void *d, un s )
            : descriptor( reinterpret_cast<pointer>( d ) ), descriptor_size( s ) {}
        public:
            const_memory_map_iterator_v1 &operator++( void )
            {
                descriptor = target::uintptr_to_ptr<value_type>(
                                    target::ptr_to_uintptr( descriptor ) + descriptor_size );
                return *this;
            }
            const_memory_map_iterator_v1 operator++( int )
            { const_memory_map_iterator_v1 cp( *this ); ++(*this); return cp; }
            
            const_memory_map_iterator_v1 &operator--( void )
            {
                descriptor = target::uintptr_to_ptr<value_type>(
                                    target::ptr_to_uintptr( descriptor ) - descriptor_size );
                return *this;
            }
            const_memory_map_iterator_v1 operator--( int )
            { const_memory_map_iterator_v1 cp( *this ); --(*this); return cp; }
            
            reference operator*( void ) const { return *descriptor; }
            pointer operator->( void ) const { return descriptor; }
            
            bool operator==( const const_memory_map_iterator_v1 &it ) const
            { return descriptor == it.descriptor; }
            bool operator!=( const const_memory_map_iterator_v1 &it ) const
            { return descriptor != it.descriptor; }
            
            bool operator<( const const_memory_map_iterator_v1 &it ) const
            { return descriptor < it.descriptor; }
            bool operator<=( const const_memory_map_iterator_v1 &it ) const
            { return descriptor <= it.descriptor; }
            bool operator>=( const const_memory_map_iterator_v1 &it ) const
            { return descriptor >= it.descriptor; }
            bool operator>( const const_memory_map_iterator_v1 &it ) const
            { return descriptor > it.descriptor; }
            
            const_memory_map_iterator_v1 &operator+=( difference_type n )
            {
                descriptor = target::uintptr_to_ptr<value_type>(
                                target::ptr_to_uintptr( descriptor ) + n * descriptor_size );
                return *this;
            }
            const_memory_map_iterator_v1 operator+( difference_type n ) const
            { const_memory_map_iterator_v1 cp( *this ); cp += n; return cp; }
            
            const_memory_map_iterator_v1 &operator-=( difference_type n )
            {
                descriptor = target::uintptr_to_ptr<value_type>(
                                target::ptr_to_uintptr( descriptor ) - n * descriptor_size );
                return *this;
            }
            const_memory_map_iterator_v1 operator-( difference_type n ) const
            { const_memory_map_iterator_v1 cp( *this ); cp -= n; return cp; }
            
            difference_type operator-( const_memory_map_iterator_v1 it ) const
            {
                return (target::ptr_to_uintptr( descriptor ) -
                        target::ptr_to_uintptr( it.descriptor )) / descriptor_size;
            }
            
            reference operator[]( difference_type n ) const
            {
                return *target::uintptr_to_ptr<value_type>(
                        target::ptr_to_uintptr( descriptor ) + n * descriptor_size );
            }
        };
        
        /** \struct memory_map
         * \brief A UEFI memory map, essentially equivalent to
         *        the result of a call to GetMemoryMap() in UEFI.
         * \warning The descriptor size and version can vary
         *          across different UEFI implementations!
         */
        struct memory_map
        {
            void *descriptors; /**< The descriptors (deliberately untyped) */
            un number_of_descriptors;
            un descriptor_size;
            uint32 descriptor_version;
            
            /** \typedef const_iterator_v1
             * \brief Iterator to traverse memory map as if
             *        EFI_MEMORY_DESCRIPTOR_VERSION was set to 1.
             */
            using const_iterator_v1 = const_memory_map_iterator_v1;
            
            /** \name v1 traversal functions
             * \{ */
            const_iterator_v1 cbegin_v1( void ) const
            { return const_iterator_v1( descriptors, descriptor_size ); }
            const_iterator_v1 cend_v1( void ) const
            { return (const_iterator_v1( descriptors, descriptor_size ) += number_of_descriptors); }
            /* \} */
        };
    }
}

#endif
    
/** \} */
