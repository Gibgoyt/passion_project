#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mdbx.h>
#include <sys/stat.h>
#include <dirent.h>

void print_hex(const char *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", (unsigned char)data[i]);
    }
}

void print_safe_string(const char *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        char c = data[i];
        if (c >= 32 && c <= 126) {
            printf("%c", c);
        } else {
            printf("\\x%02x", (unsigned char)c);
        }
    }
}

int is_mdbx_database(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0; // Path doesn't exist
    }

    if (!S_ISDIR(st.st_mode)) {
        return 0; // Not a directory
    }

    // Check for MDBX database files
    char lock_file[512];
    char data_file[512];
    snprintf(lock_file, sizeof(lock_file), "%s/mdbx.lck", path);
    snprintf(data_file, sizeof(data_file), "%s/mdbx.dat", path);

    struct stat lock_st, data_st;
    return (stat(lock_file, &lock_st) == 0 || stat(data_file, &data_st) == 0);
}

void list_databases_in_dir(const char *dir_path) {
    printf("📁 Directory: %s\n", dir_path);
    printf("   Available databases:\n");

    DIR *dir = opendir(dir_path);
    if (!dir) {
        printf("   ❌ Cannot open directory\n");
        return;
    }

    struct dirent *entry;
    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; // Skip . and ..

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (is_mdbx_database(full_path)) {
            printf("   📊 %s\n", entry->d_name);
            count++;
        }
    }

    if (count == 0) {
        printf("   📭 No MDBX databases found\n");
    }

    closedir(dir);
}

void query_key(const char *db_path, const char *key_str) {
    printf("🔍 Querying database: %s\n", db_path);
    printf("   Key: '%s'\n", key_str);

    MDBX_env *env;
    MDBX_txn *txn;
    MDBX_dbi dbi;

    int rc = mdbx_env_create(&env);
    if (rc != MDBX_SUCCESS) {
        printf("   ❌ Failed to create environment: %s\n", mdbx_strerror(rc));
        return;
    }

    rc = mdbx_env_open(env, db_path, MDBX_RDONLY, 0644);
    if (rc != MDBX_SUCCESS) {
        printf("   ❌ Failed to open database: %s\n", mdbx_strerror(rc));
        mdbx_env_close(env);
        return;
    }

    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
    if (rc != MDBX_SUCCESS) {
        printf("   ❌ Failed to begin transaction: %s\n", mdbx_strerror(rc));
        mdbx_env_close(env);
        return;
    }

    // Try to open default DBI first
    rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
    if (rc != MDBX_SUCCESS) {
        printf("   ❌ Failed to open DBI: %s\n", mdbx_strerror(rc));
        mdbx_txn_abort(txn);
        mdbx_env_close(env);
        return;
    }

    // For now, always try KV first, then DupSort if needed
    int is_dupsort = 0; // We'll detect this during the query

    // Query the key
    MDBX_val key = {(void*)key_str, strlen(key_str)};
    MDBX_val data;

    // Try regular KV lookup first
    rc = mdbx_get(txn, dbi, &key, &data);
    if (rc == MDBX_SUCCESS) {
        printf("   ✅ Key found (Key-Value):\n");
        printf("     Value (%zu bytes): ", data.iov_len);
        print_safe_string((char*)data.iov_base, data.iov_len);
        printf("\n     Hex: ");
        print_hex((char*)data.iov_base, data.iov_len);
        printf("\n");
    } else {
        // Try DupSort lookup with cursor
        MDBX_cursor *cursor;
        rc = mdbx_cursor_open(txn, dbi, &cursor);
        if (rc != MDBX_SUCCESS) {
            printf("   ❌ Failed to open cursor: %s\n", mdbx_strerror(rc));
            mdbx_txn_abort(txn);
            mdbx_env_close(env);
            return;
        }

        rc = mdbx_cursor_get(cursor, &key, &data, MDBX_SET);
        if (rc == MDBX_SUCCESS) {
            printf("   ✅ Key found (DupSort values):\n");
            int count = 0;
            do {
                count++;
                printf("     Value #%d (%zu bytes): ", count, data.iov_len);
                print_safe_string((char*)data.iov_base, data.iov_len);
                printf("\n     Hex: ");
                print_hex((char*)data.iov_base, data.iov_len);
                printf("\n");

                rc = mdbx_cursor_get(cursor, &key, &data, MDBX_NEXT_DUP);
            } while (rc == MDBX_SUCCESS);

            printf("   📊 Total values: %d\n", count);
        } else {
            printf("   ❌ Key not found: %s\n", mdbx_strerror(rc));
        }

        mdbx_cursor_close(cursor);
    }

    mdbx_txn_commit(txn);
    mdbx_env_close(env);
}

void show_all_keys(const char *db_path) {
    printf("🔍 Listing all keys in: %s\n", db_path);

    MDBX_env *env;
    MDBX_txn *txn;
    MDBX_dbi dbi;
    MDBX_cursor *cursor;

    int rc = mdbx_env_create(&env);
    if (rc != MDBX_SUCCESS) {
        printf("   ❌ Failed to create environment: %s\n", mdbx_strerror(rc));
        return;
    }

    rc = mdbx_env_open(env, db_path, MDBX_RDONLY, 0644);
    if (rc != MDBX_SUCCESS) {
        printf("   ❌ Failed to open database: %s\n", mdbx_strerror(rc));
        mdbx_env_close(env);
        return;
    }

    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
    if (rc != MDBX_SUCCESS) {
        printf("   ❌ Failed to begin transaction: %s\n", mdbx_strerror(rc));
        mdbx_env_close(env);
        return;
    }

    rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
    if (rc != MDBX_SUCCESS) {
        printf("   ❌ Failed to open DBI: %s\n", mdbx_strerror(rc));
        mdbx_txn_abort(txn);
        mdbx_env_close(env);
        return;
    }

    rc = mdbx_cursor_open(txn, dbi, &cursor);
    if (rc != MDBX_SUCCESS) {
        printf("   ❌ Failed to open cursor: %s\n", mdbx_strerror(rc));
        mdbx_txn_abort(txn);
        mdbx_env_close(env);
        return;
    }

    MDBX_val key, data;
    int count = 0;

    rc = mdbx_cursor_get(cursor, &key, &data, MDBX_FIRST);
    while (rc == MDBX_SUCCESS) {
        count++;
        printf("   Key #%d: ", count);
        print_safe_string((char*)key.iov_base, key.iov_len);
        printf("\n");

        rc = mdbx_cursor_get(cursor, &key, &data, MDBX_NEXT);
    }

    if (count == 0) {
        printf("   📭 Database is empty\n");
    } else {
        printf("   📊 Total keys: %d\n", count);
    }

    mdbx_cursor_close(cursor);
    mdbx_txn_commit(txn);
    mdbx_env_close(env);
}

void print_usage(const char *prog_name) {
    printf("Usage:\n");
    printf("  %s <database_path>                     # List all keys\n", prog_name);
    printf("  %s <database_path> <key>              # Query specific key\n", prog_name);
    printf("  %s <directory_path>                   # List databases in directory\n", prog_name);
    printf("\nExamples:\n");
    printf("  %s ./data/users                       # List all user keys\n", prog_name);
    printf("  %s ./data/users 6qqwX6QHJfF9Q0dt1...  # Get specific user\n", prog_name);
    printf("  %s ./data/email_index testuser1@...   # Lookup by email\n", prog_name);
    printf("  %s ./data                             # List all databases\n", prog_name);
}

int main(int argc, char *argv[]) {
    printf("📊 MDBX Database Query Tool\n");
    printf("===========================\n\n");

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char *path = argv[1];

    if (is_mdbx_database(path)) {
        // It's a database
        if (argc == 2) {
            // List all keys
            show_all_keys(path);
        } else if (argc == 3) {
            // Query specific key
            query_key(path, argv[2]);
        } else {
            print_usage(argv[0]);
            return 1;
        }
    } else {
        // Check if it's a directory
        struct stat st;
        if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
            list_databases_in_dir(path);
        } else {
            printf("❌ '%s' is not a database or directory\n\n", path);
            print_usage(argv[0]);
            return 1;
        }
    }

    return 0;
}