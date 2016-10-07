#/** \ingroup kernel
* \{
*
* \file kernel/memory_manager.hpp
* \brief This file declares two types of memory managers,
*        an unsynchronized and a synchronized.
*
* They are intended to be used during different stages of
* OS boot.
*/

#ifndef H_kernel_memory_manager
#define H_kernel_memory_manager

#include "memory_map.hpp"
#include "debug.hpp"

#include <algorithm>
#include <memory_resource>
#include <functional>

namespace UtopiaOS
{
    namespace kernel
    {
        /** \class memory_manager
         * \brief This class is an abstract base class
         *        enforcing a meaningful interface.
         */
        class memory_manager
        {
        public:
            virtual ~memory_manager( void ) = default;
        };
        
        /** \class unsynchronized_memory_manager
         * \brief A memory managing object from which allocators
         *        can be retrieved.
         */
        class unsynchronized_memory_manager : public virtual memory_manager
        {
        private:
            std::pmr::unsynchronized_pool_resource *memory_pool;
        public:
            // OMD stands for "occupied memory description"
            /** \brief Construct the memory manager from a memory map
             *         and data about occupied memory.
             * \tparam MemMap A kernel-usable memory map
             * \tparam InputIterator An iterator whose value_type s are
             *         memory_region s.
             * \param[in] old_memmap The old memory map
             * \param[in] omd_begin Begin of omd
             * \param[in] omd_end End of omd
             */
            template<class MemMap, class InputIterator>
            unsynchronized_memory_manager( MemMap &&old_memmap,
                                          InputIterator omd_begin, InputIterator omd_end )
            {
                const auto copy_requirement = old_memmap.copy_requirement();
                
                auto it = std::find_if( old_memmap.cbegin(), old_memmap.cend(),
                                       std::bind( &memory_descriptor::can_meet_request,
                                                 std::placeholders::_1, copy_requirement ) );
            }
            
            /** \brief Because an unsynchronized_memory_manager manages a single
             *         std::pmr::unsynchronized_pool_resource which is itself
             *         not copyable, the copy constructor has to be deleted.
             */
            unsynchronized_memory_manager( const unsynchronized_memory_manager & ) = delete;
            
            /** \brief Because an unsynchronized_memory_manager manages a single
             *         std::pmr::unsynchronized_pool_resource which is itself
             *         not copy-assignable, the copy-assign constructor has to be deleted.
             */
            unsynchronized_memory_manager &operator=( const unsynchronized_memory_manager & ) = delete;
        };
    }
}

#endif

/** \} */
