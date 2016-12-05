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
#include <array>

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
        
        /** \class const_memory_map_iterator
         * \brief An iterator template that can be used to
         *        traverse the elements of a UEFI memory map.
         */
        template<class Descriptor>
        class const_memory_map_iterator
        {
        public:
            /** \name Iterator Traits
             * \{ */
            using iterator_category = std::random_access_iterator_tag;
            using value_type = const Descriptor;
            using reference = const Descriptor &;
            using pointer = const Descriptor *;
            using difference_type = std::make_signed<un>::type;
            /** \} */
            
            pointer descriptor; /**< The current memory descriptor */
            un descriptor_size; /**< The real size of the memory descriptor */
        public:
            /** \brief Construct an iterator from data supplied by a UEFI memory map */
            const_memory_map_iterator( void *d, un s )
            : descriptor( reinterpret_cast<pointer>( d ) ), descriptor_size( s ) {}
            
            const_memory_map_iterator &operator++( void )
            {
                descriptor = target::uintptr_to_ptr<value_type>(
                                    target::ptr_to_uintptr( descriptor ) + descriptor_size );
                return *this;
            }
            const_memory_map_iterator operator++( int )
            { const_memory_map_iterator cp( *this ); ++(*this); return cp; }
            
            const_memory_map_iterator &operator--( void )
            {
                descriptor = target::uintptr_to_ptr<value_type>(
                                    target::ptr_to_uintptr( descriptor ) - descriptor_size );
                return *this;
            }
            const_memory_map_iterator operator--( int )
            { const_memory_map_iterator cp( *this ); --(*this); return cp; }
            
            reference operator*( void ) const { return *descriptor; }
            pointer operator->( void ) const { return descriptor; }
            
            bool operator==( const const_memory_map_iterator &it ) const
            { return descriptor == it.descriptor; }
            bool operator!=( const const_memory_map_iterator &it ) const
            { return descriptor != it.descriptor; }
            
            bool operator<( const const_memory_map_iterator &it ) const
            { return descriptor < it.descriptor; }
            bool operator<=( const const_memory_map_iterator &it ) const
            { return descriptor <= it.descriptor; }
            bool operator>=( const const_memory_map_iterator &it ) const
            { return descriptor >= it.descriptor; }
            bool operator>( const const_memory_map_iterator &it ) const
            { return descriptor > it.descriptor; }
            
            const_memory_map_iterator &operator+=( difference_type n )
            {
                descriptor = target::uintptr_to_ptr<value_type>(
                                target::ptr_to_uintptr( descriptor ) + n * descriptor_size );
                return *this;
            }
            const_memory_map_iterator operator+( difference_type n ) const
            { const_memory_map_iterator cp( *this ); cp += n; return cp; }
            
            const_memory_map_iterator &operator-=( difference_type n )
            {
                descriptor = target::uintptr_to_ptr<value_type>(
                                target::ptr_to_uintptr( descriptor ) - n * descriptor_size );
                return *this;
            }
            const_memory_map_iterator operator-( difference_type n ) const
            { const_memory_map_iterator cp( *this ); cp -= n; return cp; }
            
            difference_type operator-( const_memory_map_iterator it ) const
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
         * \note This structure needs to be API/ABI-stable
         *       and hence may not be changed!
         */
        struct memory_map
        {
            void *descriptors; /**< The descriptors (deliberately untyped) */
            un number_of_descriptors;
            un descriptor_size;
            uint32 descriptor_version;
            uint32 least_compatible_version; /**< The minimum version that is still compatible */
            
            /** \brief Returns the memory regions occpied by the memory map.
             * \returns The memory regions occupied by the memory_map object.
             */
            auto occupied_memory( void ) const
            {
                target::memory_region object_region = { target::ptr_to_uintptr( this ),
                    sizeof( memory_map ) };
                target::memory_region descriptor_region = { target::ptr_to_uintptr( descriptors ),
                    sizeof( number_of_descriptors * descriptor_size ) };
                
                return std::array<target::memory_region, 2>{ object_region, descriptor_region };
            }
        };
        
        /** \typedef const_memmap_iterator_v1
         * \brief Iterator to traverse a memory map as if its
         *        EFI_MEMORY_DESCRIPTOR_VERSION was set to 1.
         */
        using const_memmap_iterator_v1 = const_memory_map_iterator<memory_descriptor_v1>;
        
        /** \name v1 memory map traversal functions
         * \{ */
        const_memmap_iterator_v1 cbegin_v1( const memory_map &memmap )
        { return const_memmap_iterator_v1( memmap.descriptors, memmap.descriptor_size ); }
        const_memmap_iterator_v1 cend_v1( const memory_map &memmap )
        { return (const_memmap_iterator_v1( memmap.descriptors, memmap.descriptor_size )
                  += memmap.number_of_descriptors); }
        /* \} */
        
        /** \struct memory_region
         * \brief This struct represents a memory region
         *        in a UEFI-compatible way
         */
        struct memory_region
        {
            /** \brief The start of the memory region */
            uint64 start;
            /** \brief The size of the memory region */
            uint64 size;
            
            /** \brief Explicit conversion operator to
             *         target::memory_region for use
             *         by the kernel.
             */
            explicit operator target::memory_region( void ) const
            {
                static_assert( (std::numeric_limits<uint64>::max() <=
                                std::numeric_limits<std::uintptr_t>::max()) &&
                              (std::numeric_limits<uint64>::max() <=
                               std::numeric_limits<std::size_t>::max()),
                              "UEFI memory region and target memory region are incompatible." );
                return target::memory_region{ start, size };
            }
        };
    }
}

#endif

/** \} */
