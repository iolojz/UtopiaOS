#ifndef H_environment_memory
#define H_environment_memory

#include <common/types.hpp>

#include <iterator>

namespace UtopiaOS
{
    namespace UEFI
    {
        /********* UEFI compatible memory description types *********/
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
            
        static constexpr common::un pagesize = (common::un(1) << 12);
        
        using physical_address = common::uint64;
        using virtual_address = common::uint64;
        
        static constexpr common::uint64 EFI_MEMORY_UC      = (common::uint64(1) <<  0);
        static constexpr common::uint64 EFI_MEMORY_WC      = (common::uint64(1) <<  1);
        static constexpr common::uint64 EFI_MEMORY_WT      = (common::uint64(1) <<  2);
        static constexpr common::uint64 EFI_MEMORY_WB      = (common::uint64(1) <<  3);
        static constexpr common::uint64 EFI_MEMORY_UCE     = (common::uint64(1) <<  4);
        static constexpr common::uint64 EFI_MEMORY_WP      = (common::uint64(1) << 12);
        static constexpr common::uint64 EFI_MEMORY_RP      = (common::uint64(1) << 13);
        static constexpr common::uint64 EFI_MEMORY_XP      = (common::uint64(1) << 14);
        static constexpr common::uint64 EFI_MEMORY_RUNTIME = (common::uint64(1) << 63);
        
        struct memory_descriptor_v1
        {
            memory_type type;
            physical_address physical_start; // 4KiB aligned!
            virtual_address virtual_start; // 4KiB aligned!
            common::uint64 number_of_pages; // number of 4KiB pages
            common::uint64 attribute;
        };
            
        class const_memory_map_iterator_v1
        {
        public:
            friend class memory_map;
            
            using iterator_category = std::random_access_iterator_tag;
            using value_type = const memory_descriptor_v1;
            using reference = const memory_descriptor_v1 &;
            using pointer = const memory_descriptor_v1 *;
            using difference_type = common::sn;
            
            pointer descriptor;
            common::un descriptor_size;
            
            const_memory_map_iterator_v1( void *d, common::un s )
            : descriptor( reinterpret_cast<pointer>( d ) ), descriptor_size( s ) {}
        public:
            const_memory_map_iterator_v1 &operator++( void )
            {
                descriptor = reinterpret_cast<pointer>( reinterpret_cast<common::uintptr>( descriptor ) + descriptor_size );
                return *this;
            }
            const_memory_map_iterator_v1 operator++( int ) { const_memory_map_iterator_v1 cp( *this ); ++(*this); return cp; }
            
            const_memory_map_iterator_v1 &operator--( void )
            {
                descriptor = reinterpret_cast<pointer>( reinterpret_cast<common::uintptr>( descriptor ) - descriptor_size );
                return *this;
            }
            const_memory_map_iterator_v1 operator--( int ) { const_memory_map_iterator_v1 cp( *this ); --(*this); return cp; }
            
            value_type &operator*( void ) const { return *descriptor; }
            pointer operator->( void ) const { return descriptor; }
            
            bool operator==( const const_memory_map_iterator_v1 &it ) const { return descriptor == it.descriptor; }
            bool operator!=( const const_memory_map_iterator_v1 &it ) const { return descriptor != it.descriptor; }
            
            bool operator<( const const_memory_map_iterator_v1 &it ) const { return descriptor < it.descriptor; }
            bool operator<=( const const_memory_map_iterator_v1 &it ) const { return descriptor <= it.descriptor; }
            bool operator>=( const const_memory_map_iterator_v1 &it ) const { return descriptor >= it.descriptor; }
            bool operator>( const const_memory_map_iterator_v1 &it ) const { return descriptor > it.descriptor; }
            
            const_memory_map_iterator_v1 &operator+=( difference_type n )
            {
                descriptor = reinterpret_cast<pointer>( reinterpret_cast<common::uintptr>( descriptor ) + n * descriptor_size );
                return *this;
            }
            const_memory_map_iterator_v1 operator+( difference_type n ) const { const_memory_map_iterator_v1 cp( *this ); cp += n; return cp; }
            
            const_memory_map_iterator_v1 &operator-=( difference_type n )
            {
                descriptor = reinterpret_cast<pointer>( reinterpret_cast<common::uintptr>( descriptor ) - n * descriptor_size );
                return *this;
            }
            const_memory_map_iterator_v1 operator-( difference_type n ) const { const_memory_map_iterator_v1 cp( *this ); cp -= n; return cp; }
            
            difference_type operator-( const_memory_map_iterator_v1 it ) const
            {
                return (reinterpret_cast<common::uintptr>( descriptor ) -
                        reinterpret_cast<common::uintptr>( it.descriptor ) / descriptor_size);
            }
            
            reference operator[]( difference_type n ) const
            { return *reinterpret_cast<pointer>( reinterpret_cast<common::uintptr>( descriptor ) + n * descriptor_size ); }
        };
                
        struct memory_map
        {
            // Note that the descriptors are void pointers!
            // This is because future versions of UEFI
            // might specify larger descriptors types.
            // Beware!
            
            void *descriptors;
            common::un number_of_descriptors;
            common::un descriptor_size;
            common::uint32 descriptor_version;
            
            using const_iterator_v1 = const_memory_map_iterator_v1;
            
            const_iterator_v1 cbegin_v1( void ) const { return const_iterator_v1( descriptors, descriptor_size ); }
            const_iterator_v1 cend_v1( void ) const { return (const_iterator_v1( descriptors, descriptor_size ) += number_of_descriptors); }
        };
    }
}

#endif
