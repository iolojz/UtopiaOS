#ifndef H_kernel_kernel_main
#define H_kernel_kernel_main

namespace UtopiaOS
{
    class environment;
    
    namespace Kernel
    {
        [[noreturn]] void kernel_main( const environment * );
    }
}

#endif
