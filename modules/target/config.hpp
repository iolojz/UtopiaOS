#ifndef H_target_config
#define H_target_config

#define JAYZOS_ENABLE_DEBUG_ASSERTS 1

#define JAYZOS_KERNEL_PAGESIZE (1 << 12)

#define JAYZOS_TRAP() __builtin_trap()

#define JAYZOS_ALLOCA_WITH_ALIGN( size, alignment ) __builtin_alloca_with_align( size, alignment )

#endif /* H_common_target_headers */
