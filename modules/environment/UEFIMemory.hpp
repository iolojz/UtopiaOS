/** \ingroup environment
 * \{
 *
 * \file environment/UEFIMemory.hpp
 * \brief This file contains basic memory-related
 *        types and constants modelled to be UEFI
 *        compatible.
 */

#ifndef H_environment_memory
#define H_environment_memory

#include <common/types.hpp>

#include <iterator>

namespace UtopiaOS
{
    /** \namespace UtopiaOS::UEFI
     * \brief This namespace contains types and constants
     *        geared towards a UEFI compatible interface
     */
    namespace UEFI
    {
        /** \enum memory_type
         * \brief Equivalent to the UEFI type EFI_MEMORY_TYPE */
        enum class memory_type : common::uint32
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
        static constexpr common::un pagesize = (common::un(1) << 12);
        
        /** \typedef physical_address
         * \brief Equivalent to the UEFI type EFI_PHYSICAL_ADDRESS
         */
        using physical_address = common::uint64;
        /** \typedef physical_address
         * \brief Equivalent to the UEFI type EFI_VIRTUAL_ADDRESS
         */
        using virtual_address = common::uint64;
        
        /** \name Memory Attribute Definitions 
         * \brief Constants equivalent to the UEFI memory attribute definitions
         * \{ */
        static constexpr common::uint64 EFI_MEMORY_UC      = (common::uint64(1) <<  0);
        static constexpr common::uint64 EFI_MEMORY_WC      = (common::uint64(1) <<  1);
        static constexpr common::uint64 EFI_MEMORY_WT      = (common::uint64(1) <<  2);
        static constexpr common::uint64 EFI_MEMORY_WB      = (common::uint64(1) <<  3);
        static constexpr common::uint64 EFI_MEMORY_UCE     = (common::uint64(1) <<  4);
        static constexpr common::uint64 EFI_MEMORY_WP      = (common::uint64(1) << 12);
        static constexpr common::uint64 EFI_MEMORY_RP      = (common::uint64(1) << 13);
        static constexpr common::uint64 EFI_MEMORY_XP      = (common::uint64(1) << 14);
        static constexpr common::uint64 EFI_MEMORY_RUNTIME = (common::uint64(1) << 63);
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
            common::uint64 number_of_pages; /**< number of 4KiB pages */
            common::uint64 attribute;
        };
        
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
            friend class memory_map;
            
            /** \name Iterator Traits
             * \{ */
            using iterator_category = std::random_access_iterator_tag;
            using value_type = const memory_descriptor_v1;
            using reference = const memory_descriptor_v1 &;
            using pointer = const memory_descriptor_v1 *;
            using difference_type = common::sn;
            /** \} */
            
            pointer descriptor; /**< The current memory descriptor */
            common::un descriptor_size; /**< The real size of the memory descriptor */
            
        private:
            /** \brief Construct an iterator from data supplied by a UEFI memory map */
            const_memory_map_iterator_v1( void *d, common::un s )
            : descriptor( reinterpret_cast<pointer>( d ) ), descriptor_size( s ) {}
        public:
            const_memory_map_iterator_v1 &operator++( void )
            {
                descriptor = common::uintptr_to_ptr<value_type>( common::ptr_to_uintptr( descriptor ) + descriptor_size );
                return *this;
            }
            const_memory_map_iterator_v1 operator++( int )
            { const_memory_map_iterator_v1 cp( *this ); ++(*this); return cp; }
            
            const_memory_map_iterator_v1 &operator--( void )
            {
                descriptor = common::uintptr_to_ptr<value_type>( common::ptr_to_uintptr( descriptor ) - descriptor_size );
                return *this;
            }
            const_memory_map_iterator_v1 operator--( int )
            { const_memory_map_iterator_v1 cp( *this ); --(*this); return cp; }
            
            value_type &operator*( void ) const { return *descriptor; }
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
                descriptor = common::uintptr_to_ptr<value_type>( common::ptr_to_uintptr( descriptor ) + n * descriptor_size );
                return *this;
            }
            const_memory_map_iterator_v1 operator+( difference_type n ) const
            { const_memory_map_iterator_v1 cp( *this ); cp += n; return cp; }
            
            const_memory_map_iterator_v1 &operator-=( difference_type n )
            {
                descriptor = common::uintptr_to_ptr<value_type>( common::ptr_to_uintptr( descriptor ) - n * descriptor_size );
                return *this;
            }
            const_memory_map_iterator_v1 operator-( difference_type n ) const
            { const_memory_map_iterator_v1 cp( *this ); cp -= n; return cp; }
            
            difference_type operator-( const_memory_map_iterator_v1 it ) const
            {
                return (common::ptr_to_uintptr( descriptor ) - common::ptr_to_uintptr( it.descriptor ) / descriptor_size);
            }
            
            reference operator[]( difference_type n ) const
            { return *common::uintptr_to_ptr<value_type>( common::ptr_to_uintptr( descriptor ) + n * descriptor_size ); }
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
            common::un number_of_descriptors;
            common::un descriptor_size;
            common::uint32 descriptor_version;
            
            /** \typedef const_iterator_v1
             * \brief Iterator to traverse memory map as if
             *        EFI_MEMORY_DESCRIPTOR_VERSION was set to 1.
             */
            using const_iterator_v1 = const_memory_map_iterator_v1;
            
            /** \name v1 traversal functions
             * \{ */
            const_iterator_v1 cbegin_v1( void ) const { return const_iterator_v1( descriptors, descriptor_size ); }
            const_iterator_v1 cend_v1( void ) const { return (const_iterator_v1( descriptors, descriptor_size ) += number_of_descriptors); }
            /* \} */
        };
    }
}

#endif
    
/** \} */
