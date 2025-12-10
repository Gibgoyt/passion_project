#ifndef MEMORY_VALIDATION_H
#define MEMORY_VALIDATION_H

#include "page_allocator.h"
#include "jwt_storage.h"
#include "../auth_lib/jwt_rs256.h"
#include <stddef.h>
#include <stdint.h>

/**
 * Comprehensive Memory Validation System
 *
 * Provides both compile-time and runtime validation for memory management
 * on Oracle A1.Flex ARM64 architecture. Ensures compatibility between:
 * - Page allocation system (4KB pages)
 * - JWT storage requirements (up to 2048 bytes + Bearer prefix)
 * - Cache line alignment (64-byte boundaries)
 * - Authentication library expectations
 */

// ============================================================================
// COMPILE-TIME VALIDATIONS
// ============================================================================

// Oracle A1.Flex ARM64 architecture validations
_Static_assert(ARM64_PAGE_SIZE == 4096,
               "Page size must be 4KB on Oracle A1.Flex ARM64");
_Static_assert(ARM64_CACHE_LINE_SIZE == 64,
               "Cache line must be 64 bytes on Oracle A1.Flex ARM64");
_Static_assert((ARM64_PAGE_SIZE % ARM64_CACHE_LINE_SIZE) == 0,
               "Page size must be multiple of cache line size");

// JWT library compatibility validations
_Static_assert(JWT_MAX_LENGTH > 0,
               "JWT_MAX_LENGTH must be defined and positive");
_Static_assert(JWT_MAX_LENGTH <= (8 * ARM64_PAGE_SIZE),
               "JWT_MAX_LENGTH must not exceed reasonable page allocation");

// Buffer size compatibility checks
#define REQUIRED_AUTH_BUFFER_SIZE (JWT_MAX_LENGTH + 16)  // JWT + "Bearer " + margin

_Static_assert(REQUIRED_AUTH_BUFFER_SIZE <= (2 * ARM64_PAGE_SIZE),
               "Required authorization buffer size should fit in 2 pages");

// Ensure JWT storage structure alignment
_Static_assert((sizeof(jwt_storage_t) % 8) == 0,
               "jwt_storage_t must be 8-byte aligned");
_Static_assert(sizeof(jwt_storage_t) <= ARM64_CACHE_LINE_SIZE * 2,
               "jwt_storage_t should fit within cache-friendly boundary");

// Magic number validations
_Static_assert(JWT_STORAGE_MAGIC != 0,
               "JWT storage magic must not be zero");
_Static_assert((JWT_STORAGE_MAGIC & 0xFFFF) != 0,
               "JWT storage magic must have non-zero lower bits");

// Validation flag sanity checks
_Static_assert((JWT_VALIDATE_ALL & JWT_VALIDATE_NULL_TERMINATION) != 0,
               "JWT_VALIDATE_ALL must include null termination check");
_Static_assert((JWT_VALIDATE_ALL & JWT_VALIDATE_NO_EMBEDDED_NULLS) != 0,
               "JWT_VALIDATE_ALL must include embedded null check");
_Static_assert((JWT_VALIDATE_ALL & JWT_VALIDATE_ASCII_ONLY) != 0,
               "JWT_VALIDATE_ALL must include ASCII validation");
_Static_assert((JWT_VALIDATE_ALL & JWT_VALIDATE_LENGTH_LIMITS) != 0,
               "JWT_VALIDATE_ALL must include length validation");

// Page allocation error code validations
_Static_assert(PAGE_ALLOC_SUCCESS == 0,
               "Success code must be zero for standard conventions");
_Static_assert(PAGE_ALLOC_ERROR_INVALID_PARAM < 0,
               "Error codes must be negative");

// JWT storage error code validations
_Static_assert(JWT_STORAGE_SUCCESS == 0,
               "Success code must be zero for standard conventions");
_Static_assert(JWT_STORAGE_ERROR_INVALID_PARAM < 0,
               "Error codes must be negative");

// ============================================================================
// RUNTIME VALIDATION FUNCTIONS
// ============================================================================

/**
 * Validate Oracle A1.Flex architecture assumptions at runtime
 * @return 1 if all assumptions are correct, 0 otherwise
 */
int validate_architecture_runtime(void);

/**
 * Validate JWT library compatibility at runtime
 * @return 1 if compatible, 0 if incompatible
 */
int validate_jwt_library_compatibility(void);

/**
 * Validate page allocator functionality
 * @return 1 if working correctly, 0 if issues detected
 */
int validate_page_allocator(void);

/**
 * Validate JWT storage functionality
 * @return 1 if working correctly, 0 if issues detected
 */
int validate_jwt_storage(void);

/**
 * Comprehensive system validation
 * @return 1 if all validations pass, 0 if any fail
 */
int validate_memory_system(void);

/**
 * Calculate optimal buffer size for authorization headers
 * @return Recommended buffer size in bytes
 */
size_t calculate_optimal_auth_buffer_size(void);

/**
 * Validate pointer alignment
 * @param ptr Pointer to check
 * @param alignment Required alignment (must be power of 2)
 * @return 1 if aligned, 0 if not
 */
int validate_pointer_alignment(const void* ptr, size_t alignment);

/**
 * Validate page boundaries
 * @param ptr Pointer to check
 * @param size Size of data
 * @return 1 if within page boundaries, 0 if crosses pages unsafely
 */
int validate_page_boundaries(const void* ptr, size_t size);

/**
 * Validate cache line boundaries for performance
 * @param ptr Pointer to check
 * @param size Size of data
 * @return 1 if cache-friendly, 0 if suboptimal
 */
int validate_cache_boundaries(const void* ptr, size_t size);

/**
 * Check memory region for corruption patterns
 * @param ptr Memory region to check
 * @param size Size of region
 * @return 1 if appears uncorrupted, 0 if potential corruption detected
 */
int validate_memory_integrity(const void* ptr, size_t size);

/**
 * Validate JWT token format without parsing
 * @param token Token string to validate
 * @param length Token length
 * @return 1 if basic format is valid, 0 if invalid
 */
int validate_jwt_token_format(const char* token, size_t length);

/**
 * Print comprehensive validation report
 */
void print_validation_report(void);

/**
 * Print memory layout information
 * @param ptr Memory region to analyze
 * @param size Size of region
 */
void print_memory_layout(const void* ptr, size_t size);

// ============================================================================
// DEBUGGING AND MONITORING
// ============================================================================

/**
 * Memory validation statistics
 */
typedef struct {
    uint32_t architecture_checks_passed;
    uint32_t architecture_checks_failed;
    uint32_t page_allocations_validated;
    uint32_t page_allocation_failures;
    uint32_t jwt_storage_validations;
    uint32_t jwt_storage_failures;
    uint32_t alignment_checks_performed;
    uint32_t alignment_violations_found;
    uint32_t cache_optimality_checks;
    uint32_t cache_suboptimal_cases;
} memory_validation_stats_t;

/**
 * Get memory validation statistics
 * @param stats Pointer to statistics structure to fill
 */
void get_memory_validation_stats(memory_validation_stats_t* stats);

/**
 * Reset memory validation statistics
 */
void reset_memory_validation_stats(void);

/**
 * Print memory validation statistics
 */
void print_memory_validation_stats(void);

// ============================================================================
// COMPILE-TIME CONFIGURATION VALIDATION
// ============================================================================

// Validate that required preprocessor definitions are consistent
#ifndef JWT_MAX_LENGTH
#error "JWT_MAX_LENGTH must be defined before including memory_validation.h"
#endif

#ifndef ARM64_PAGE_SIZE
#error "ARM64_PAGE_SIZE must be defined before including memory_validation.h"
#endif

#ifndef ARM64_CACHE_LINE_SIZE
#error "ARM64_CACHE_LINE_SIZE must be defined before including memory_validation.h"
#endif

// Ensure backwards compatibility with existing code
#ifdef AUTHORIZATION_BUFFER_SIZE
_Static_assert(AUTHORIZATION_BUFFER_SIZE >= REQUIRED_AUTH_BUFFER_SIZE,
               "Existing AUTHORIZATION_BUFFER_SIZE is too small for JWT tokens");
#endif

// Warning for potentially inefficient configurations
#if JWT_MAX_LENGTH < 1024
#warning "JWT_MAX_LENGTH is quite small - modern JWTs may not fit"
#endif

#if JWT_MAX_LENGTH > (16 * ARM64_PAGE_SIZE)
#warning "JWT_MAX_LENGTH is very large - consider if this is necessary"
#endif

#endif // MEMORY_VALIDATION_H