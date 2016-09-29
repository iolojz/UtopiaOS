#ifndef H_kernel_kernel_main
#define H_kernel_kernel_main

namespace JayZOS
{
    class environment;
    
    namespace Kernel
    {
        void kernel_main( const environment * ) [[noreturn]];
    }
}

#endif
