#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mdbx.h>

#define DB_PATH "./test_dupsort_data"

void test_dupsort() {
    MDBX_env *env;
    MDBX_dbi dbi;
    MDBX_txn *txn;
    MDBX_val key, val;
    MDBX_cursor *cursor;
    int rc;
    
    printf("=== Dupsort Test ===\n\n");
    
    // Create environment
    rc = mdbx_env_create(&env);
    assert(rc == MDBX_SUCCESS);
    
    // Open environment
    rc = mdbx_env_open(env, DB_PATH, MDBX_NOSUBDIR | MDBX_COALESCE, 0664);
    assert(rc == MDBX_SUCCESS);
    
    // Open database with DUPSORT flag
    rc = mdbx_txn_begin(env, NULL, 0, &txn);
    assert(rc == MDBX_SUCCESS);
    
    rc = mdbx_dbi_open(txn, NULL, MDBX_CREATE | MDBX_DUPSORT, &dbi);
    assert(rc == MDBX_SUCCESS);
    
    rc = mdbx_txn_commit(txn);
    assert(rc == MDBX_SUCCESS);
    
    // Write: Insert multiple values for the same key
    printf("1. Inserting duplicate values for key 'user:1':\n");
    rc = mdbx_txn_begin(env, NULL, 0, &txn);
    assert(rc == MDBX_SUCCESS);
    
    key.iov_base = "user:1";
    key.iov_len = strlen("user:1");
    
    // Insert values in random order - they'll be sorted automatically
    const char *values[] = {"zebra", "alpha", "delta", "beta"};
    for (int i = 0; i < 4; i++) {
        val.iov_base = (void*)values[i];
        val.iov_len = strlen(values[i]);
        
        rc = mdbx_put(txn, dbi, &key, &val, 0);
        assert(rc == MDBX_SUCCESS);
        printf("   - Inserted: %s\n", values[i]);
    }
    
    rc = mdbx_txn_commit(txn);
    assert(rc == MDBX_SUCCESS);
    
    // Read: Retrieve all values for the key (should be sorted)
    printf("\n2. Reading values (should be sorted):\n");
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
    assert(rc == MDBX_SUCCESS);
    
    rc = mdbx_cursor_open(txn, dbi, &cursor);
    assert(rc == MDBX_SUCCESS);
    
    key.iov_base = "user:1";
    key.iov_len = strlen("user:1");
    
    // Position cursor at the key
    rc = mdbx_cursor_get(cursor, &key, &val, MDBX_SET);
    assert(rc == MDBX_SUCCESS);
    
    printf("   Key: %.*s\n", (int)key.iov_len, (char*)key.iov_base);
    printf("   Values:\n");
    
    // Iterate through all duplicate values
    do {
        printf("     - %.*s\n", (int)val.iov_len, (char*)val.iov_base);
        rc = mdbx_cursor_get(cursor, &key, &val, MDBX_NEXT_DUP);
    } while (rc == MDBX_SUCCESS);
    
    // Count duplicates
    size_t count;
    rc = mdbx_cursor_count(cursor, &count);
    assert(rc == MDBX_SUCCESS);
    printf("\n   Total duplicate count: %zu\n", count);
    assert(count == 4);
    
    mdbx_cursor_close(cursor);
    mdbx_txn_abort(txn);
    
    // Cleanup
    mdbx_dbi_close(env, dbi);
    mdbx_env_close(env);
    
    printf("\n✓ Test passed!\n");
}

int main() {
    test_dupsort();
    return 0;
}