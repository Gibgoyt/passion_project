#ifndef MEMORY_VALIDATION_H
#define MEMORY_VALIDATION_H

#include "page_allocator.h"
#include "jwt_storage.h"

/**
 * Memory Validation System - Mac M1 2022 Version
 *
 * Validates memory system compatibility and performance
 * for Apple Silicon M1/M2 processors.
 */

/**
 * Validate Mac M1 memory system
 * @return 0 if all validations pass, -1 if any fail
 */
int mac_validate_memory_system(void);

/**
 * Validate Mac M1 page allocator
 * @return 0 if validation passes, -1 if fails
 */
int mac_validate_page_allocator(void);

/**
 * Validate Mac M1 JWT storage system
 * @return 0 if validation passes, -1 if fails
 */
int mac_validate_jwt_storage(void);

/**
 * Print Mac M1 memory system information
 */
void mac_print_memory_info(void);

/**
 * Test Mac M1 memory performance
 * @return 0 on success, -1 on failure
 */
int mac_test_memory_performance(void);

#endif // MEMORY_VALIDATION_H