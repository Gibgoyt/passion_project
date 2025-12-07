#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <mdbx.h>

int main() {
    printf("🔍 Testing MDBX database operations...\n");

    // Create data directory
    struct stat st = {0};
    if (stat("test_data", &st) == -1) {
        if (mkdir("test_data", 0755) != 0) {
            printf("❌ Failed to create test_data directory\n");
            return 1;
        }
    }

    MDBX_env *env = NULL;
    int rc;

    // Create environment
    rc = mdbx_env_create(&env);
    printf("📋 mdbx_env_create: %d (%s)\n", rc, mdbx_strerror(rc));
    if (rc != MDBX_SUCCESS) return 1;

    // Set map size
    rc = mdbx_env_set_mapsize(env, 64 * 1024 * 1024);
    printf("📋 mdbx_env_set_mapsize: %d (%s)\n", rc, mdbx_strerror(rc));
    if (rc != MDBX_SUCCESS) {
        mdbx_env_close(env);
        return 1;
    }

    // Open database
    rc = mdbx_env_open(env, "test_data", 0, 0664);
    printf("📋 mdbx_env_open: %d (%s)\n", rc, mdbx_strerror(rc));
    if (rc != MDBX_SUCCESS) {
        mdbx_env_close(env);
        return 1;
    }

    printf("✅ MDBX database opened successfully!\n");

    // Test a simple transaction
    MDBX_txn *txn;
    MDBX_dbi dbi;

    rc = mdbx_txn_begin(env, NULL, 0, &txn);
    printf("📋 mdbx_txn_begin: %d (%s)\n", rc, mdbx_strerror(rc));
    if (rc == MDBX_SUCCESS) {
        rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
        printf("📋 mdbx_dbi_open: %d (%s)\n", rc, mdbx_strerror(rc));
        if (rc == MDBX_SUCCESS) {
            printf("✅ MDBX transaction and DBI opened successfully!\n");
        }
        mdbx_txn_abort(txn);
    }

    mdbx_env_close(env);
    printf("✅ MDBX test completed\n");
    return 0;
}