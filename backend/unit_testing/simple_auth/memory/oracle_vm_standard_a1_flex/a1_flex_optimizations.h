#ifndef A1_FLEX_OPTIMIZATIONS_H
#define A1_FLEX_OPTIMIZATIONS_H

#include "page_allocator.h"
#include "jwt_storage.h"
#include <stddef.h>
#include <stdint.h>

/**
 * Oracle A1.Flex Architecture Specific Optimizations
 *
 * This module provides architecture-specific optimizations for:
 * - Ampere Altra ARM64 CPU optimizations
 * - Memory prefetching strategies for ARM64
 * - Cache-friendly data structure layout
 * - Performance optimization for JWT processing
 */

// Oracle A1.Flex CPU specifications
#define A1_FLEX_L1_ICACHE_SIZE    (64 * 1024)     // 64KB instruction cache
#define A1_FLEX_L1_DCACHE_SIZE    (64 * 1024)     // 64KB data cache
#define A1_FLEX_L2_CACHE_SIZE     (1024 * 1024)   // 1MB L2 cache per core
#define A1_FLEX_L3_CACHE_SIZE     (32 * 1024 * 1024) // 32MB L3 cache shared

// ARM64 prefetch instructions (if supported by compiler)
#ifdef __aarch64__
#define ARM64_PREFETCH_READ(addr)   __builtin_prefetch(addr, 0, 3)
#define ARM64_PREFETCH_WRITE(addr)  __builtin_prefetch(addr, 1, 3)
#else
#define ARM64_PREFETCH_READ(addr)   ((void)0)
#define ARM64_PREFETCH_WRITE(addr)  ((void)0)
#endif

// Memory barrier operations for ARM64
#ifdef __aarch64__
#define ARM64_MEMORY_BARRIER()      __asm__ __volatile__("dmb sy" ::: "memory")
#define ARM64_READ_BARRIER()        __asm__ __volatile__("dmb ld" ::: "memory")
#define ARM64_WRITE_BARRIER()       __asm__ __volatile__("dmb st" ::: "memory")
#else
#define ARM64_MEMORY_BARRIER()      __sync_synchronize()
#define ARM64_READ_BARRIER()        __sync_synchronize()
#define ARM64_WRITE_BARRIER()       __sync_synchronize()
#endif

/**
 * Initialize A1.Flex optimizations
 * @return 0 on success, -1 on failure
 */
int a1_flex_init_optimizations(void);

/**
 * Create cache-optimized JWT storage for A1.Flex
 * @param max_token_size Maximum token size
 * @return Optimized JWT storage or NULL on failure
 */
jwt_storage_t* a1_flex_create_optimized_jwt_storage(size_t max_token_size);

/**
 * Prefetch JWT token data for faster access
 * @param storage JWT storage to prefetch
 */
void a1_flex_prefetch_jwt_token(jwt_storage_t* storage);

/**
 * Optimize memory layout for sequential JWT processing
 * @param storages Array of JWT storage instances
 * @param count Number of storage instances
 */
void a1_flex_optimize_jwt_batch_layout(jwt_storage_t** storages, size_t count);

/**
 * Get optimal buffer size for A1.Flex cache hierarchy
 * @param data_size Expected data size
 * @return Recommended buffer size aligned to cache boundaries
 */
size_t a1_flex_get_optimal_buffer_size(size_t data_size);

/**
 * Allocate memory with A1.Flex cache optimization
 * @param size Size to allocate
 * @param cache_level Target cache level (1, 2, or 3)
 * @return Cache-optimized allocation or NULL
 */
void* a1_flex_cache_optimized_alloc(size_t size, int cache_level);

/**
 * Free cache-optimized allocation
 * @param ptr Pointer to free
 * @param size Original size
 */
void a1_flex_cache_optimized_free(void* ptr, size_t size);

/**
 * Check if memory region is optimally placed for A1.Flex
 * @param ptr Memory pointer
 * @param size Memory size
 * @return 1 if optimal, 0 if suboptimal
 */
int a1_flex_is_memory_placement_optimal(void* ptr, size_t size);

/**
 * Optimize existing JWT storage for A1.Flex performance
 * @param storage JWT storage to optimize
 * @return 0 on success, -1 on failure
 */
int a1_flex_optimize_jwt_storage_placement(jwt_storage_t* storage);

/**
 * Check A1.Flex specific CPU features and capabilities
 * @return Bitmask of available features
 */
uint32_t a1_flex_check_cpu_features(void);

// CPU feature flags
#define A1_FLEX_FEATURE_PREFETCH     (1 << 0)
#define A1_FLEX_FEATURE_MEMORY_TAGGING (1 << 1)
#define A1_FLEX_FEATURE_LSE          (1 << 2)  // Large System Extensions
#define A1_FLEX_FEATURE_SVE          (1 << 3)  // Scalable Vector Extension

#endif // A1_FLEX_OPTIMIZATIONS_H