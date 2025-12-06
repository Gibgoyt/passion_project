#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <mdbx.h>

#define PORT 9999
#define DB_PATH "./data"
#define MAX_MSG 1024

MDBX_env *env = NULL;
MDBX_dbi dbi;
int server_sock = -1;

void cleanup_handler(int sig) {
    printf("\nShutting down server...\n");
    if (env) {
        mdbx_dbi_close(env, dbi);
        mdbx_env_close(env);
    }
    if (server_sock >= 0) {
        close(server_sock);
    }
    exit(0);
}

int init_db() {
    int rc;
    
    rc = mdbx_env_create(&env);
    if (rc != MDBX_SUCCESS) {
        fprintf(stderr, "mdbx_env_create: %s\n", mdbx_strerror(rc));
        return -1;
    }
    
    rc = mdbx_env_set_geometry(env, 
        -1,
        -1,
        10UL * 1024 * 1024 * 1024,
        -1,
        -1,
        -1);
    
    if (rc != MDBX_SUCCESS) {
        fprintf(stderr, "mdbx_env_set_geometry: %s\n", mdbx_strerror(rc));
        return -1;
    }
    
    rc = mdbx_env_open(env, DB_PATH, MDBX_NOSUBDIR | MDBX_LIFORECLAIM, 0664);
    if (rc != MDBX_SUCCESS) {
        fprintf(stderr, "mdbx_env_open: %s\n", mdbx_strerror(rc));
        return -1;
    }
    
    MDBX_txn *txn;
    rc = mdbx_txn_begin(env, NULL, 0, &txn);
    if (rc != MDBX_SUCCESS) {
        fprintf(stderr, "mdbx_txn_begin: %s\n", mdbx_strerror(rc));
        return -1;
    }
    
    rc = mdbx_dbi_open(txn, NULL, MDBX_CREATE | MDBX_DUPSORT, &dbi);
    if (rc != MDBX_SUCCESS) {
        fprintf(stderr, "mdbx_dbi_open: %s\n", mdbx_strerror(rc));
        mdbx_txn_abort(txn);
        return -1;
    }
    
    rc = mdbx_txn_commit(txn);
    if (rc != MDBX_SUCCESS) {
        fprintf(stderr, "mdbx_txn_commit: %s\n", mdbx_strerror(rc));
        return -1;
    }
    
    printf("Dupsort database initialized at %s\n", DB_PATH);
    return 0;
}

int handle_add(const char *key, const char *value, char *response) {
    MDBX_txn *txn;
    MDBX_val k, v;
    int rc;
    
    rc = mdbx_txn_begin(env, NULL, 0, &txn);
    if (rc != MDBX_SUCCESS) {
        snprintf(response, MAX_MSG, "ERROR: txn_begin failed\n");
        return -1;
    }
    
    k.iov_base = (void*)key;
    k.iov_len = strlen(key);
    v.iov_base = (void*)value;
    v.iov_len = strlen(value);
    
    rc = mdbx_put(txn, dbi, &k, &v, 0);
    if (rc != MDBX_SUCCESS) {
        snprintf(response, MAX_MSG, "ERROR: %s\n", mdbx_strerror(rc));
        mdbx_txn_abort(txn);
        return -1;
    }
    
    rc = mdbx_txn_commit(txn);
    if (rc != MDBX_SUCCESS) {
        snprintf(response, MAX_MSG, "ERROR: commit failed\n");
        return -1;
    }
    
    snprintf(response, MAX_MSG, "OK: Added %s to %s\n", value, key);
    return 0;
}

int handle_get(const char *key, char *response) {
    MDBX_txn *txn;
    MDBX_val k, v;
    MDBX_cursor *cursor;
    int rc;
    
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
    if (rc != MDBX_SUCCESS) {
        snprintf(response, MAX_MSG, "ERROR: txn_begin failed\n");
        return -1;
    }
    
    rc = mdbx_cursor_open(txn, dbi, &cursor);
    if (rc != MDBX_SUCCESS) {
        snprintf(response, MAX_MSG, "ERROR: cursor_open failed\n");
        mdbx_txn_abort(txn);
        return -1;
    }
    
    k.iov_base = (void*)key;
    k.iov_len = strlen(key);
    
    rc = mdbx_cursor_get(cursor, &k, &v, MDBX_SET);
    if (rc == MDBX_NOTFOUND) {
        snprintf(response, MAX_MSG, "ERROR: Key not found\n");
        mdbx_cursor_close(cursor);
        mdbx_txn_abort(txn);
        return -1;
    } else if (rc != MDBX_SUCCESS) {
        snprintf(response, MAX_MSG, "ERROR: %s\n", mdbx_strerror(rc));
        mdbx_cursor_close(cursor);
        mdbx_txn_abort(txn);
        return -1;
    }
    
    // Build response with all duplicate values
    strcpy(response, "OK:\n");
    do {
        char value_line[512];
        snprintf(value_line, sizeof(value_line), "%.*s\n", (int)v.iov_len, (char*)v.iov_base);
        
        if (strlen(response) + strlen(value_line) < MAX_MSG - 1) {
            strcat(response, value_line);
        } else {
            strcat(response, "...\n");
            break;
        }
        
        rc = mdbx_cursor_get(cursor, &k, &v, MDBX_NEXT_DUP);
    } while (rc == MDBX_SUCCESS);
    
    mdbx_cursor_close(cursor);
    mdbx_txn_abort(txn);
    return 0;
}

int handle_count(const char *key, char *response) {
    MDBX_txn *txn;
    MDBX_val k, v;
    MDBX_cursor *cursor;
    int rc;
    
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
    if (rc != MDBX_SUCCESS) {
        snprintf(response, MAX_MSG, "ERROR: txn_begin failed\n");
        return -1;
    }
    
    rc = mdbx_cursor_open(txn, dbi, &cursor);
    if (rc != MDBX_SUCCESS) {
        snprintf(response, MAX_MSG, "ERROR: cursor_open failed\n");
        mdbx_txn_abort(txn);
        return -1;
    }
    
    k.iov_base = (void*)key;
    k.iov_len = strlen(key);
    
    rc = mdbx_cursor_get(cursor, &k, &v, MDBX_SET);
    if (rc == MDBX_NOTFOUND) {
        snprintf(response, MAX_MSG, "OK: 0\n");
        mdbx_cursor_close(cursor);
        mdbx_txn_abort(txn);
        return 0;
    } else if (rc != MDBX_SUCCESS) {
        snprintf(response, MAX_MSG, "ERROR: %s\n", mdbx_strerror(rc));
        mdbx_cursor_close(cursor);
        mdbx_txn_abort(txn);
        return -1;
    }
    
    size_t count;
    rc = mdbx_cursor_count(cursor, &count);
    if (rc != MDBX_SUCCESS) {
        snprintf(response, MAX_MSG, "ERROR: count failed\n");
        mdbx_cursor_close(cursor);
        mdbx_txn_abort(txn);
        return -1;
    }
    
    snprintf(response, MAX_MSG, "OK: %zu\n", count);
    mdbx_cursor_close(cursor);
    mdbx_txn_abort(txn);
    return 0;
}

int handle_del(const char *key, char *response) {
    MDBX_txn *txn;
    MDBX_val k;
    int rc;
    
    rc = mdbx_txn_begin(env, NULL, 0, &txn);
    if (rc != MDBX_SUCCESS) {
        snprintf(response, MAX_MSG, "ERROR: txn_begin failed\n");
        return -1;
    }
    
    k.iov_base = (void*)key;
    k.iov_len = strlen(key);
    
    rc = mdbx_del(txn, dbi, &k, NULL);
    if (rc == MDBX_SUCCESS) {
        mdbx_txn_commit(txn);
        snprintf(response, MAX_MSG, "OK: Deleted %s\n", key);
        return 0;
    } else if (rc == MDBX_NOTFOUND) {
        mdbx_txn_abort(txn);
        snprintf(response, MAX_MSG, "ERROR: Key not found\n");
        return -1;
    } else {
        mdbx_txn_abort(txn);
        snprintf(response, MAX_MSG, "ERROR: %s\n", mdbx_strerror(rc));
        return -1;
    }
}

int handle_delval(const char *key, const char *value, char *response) {
    MDBX_txn *txn;
    MDBX_val k, v;
    int rc;
    
    rc = mdbx_txn_begin(env, NULL, 0, &txn);
    if (rc != MDBX_SUCCESS) {
        snprintf(response, MAX_MSG, "ERROR: txn_begin failed\n");
        return -1;
    }
    
    k.iov_base = (void*)key;
    k.iov_len = strlen(key);
    v.iov_base = (void*)value;
    v.iov_len = strlen(value);
    
    rc = mdbx_del(txn, dbi, &k, &v);
    if (rc == MDBX_SUCCESS) {
        mdbx_txn_commit(txn);
        snprintf(response, MAX_MSG, "OK: Deleted value %s from %s\n", value, key);
        return 0;
    } else if (rc == MDBX_NOTFOUND) {
        mdbx_txn_abort(txn);
        snprintf(response, MAX_MSG, "ERROR: Key/value not found\n");
        return -1;
    } else {
        mdbx_txn_abort(txn);
        snprintf(response, MAX_MSG, "ERROR: %s\n", mdbx_strerror(rc));
        return -1;
    }
}

void handle_client(int client_sock) {
    char buffer[MAX_MSG];
    char response[MAX_MSG];
    
    int n = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) return;
    
    buffer[n] = '\0';
    
    char *newline = strchr(buffer, '\n');
    if (newline) *newline = '\0';
    
    printf("Received: %s\n", buffer);
    
    char cmd[16], key[256], value[512];
    int parsed = sscanf(buffer, "%15s %255s %511[^\n]", cmd, key, value);
    
    if (parsed < 2) {
        snprintf(response, MAX_MSG, "ERROR: Invalid command format\n");
    } else if (strcmp(cmd, "ADD") == 0 && parsed == 3) {
        handle_add(key, value, response);
    } else if (strcmp(cmd, "GET") == 0) {
        handle_get(key, response);
    } else if (strcmp(cmd, "COUNT") == 0) {
        handle_count(key, response);
    } else if (strcmp(cmd, "DEL") == 0) {
        handle_del(key, response);
    } else if (strcmp(cmd, "DELVAL") == 0 && parsed == 3) {
        handle_delval(key, value, response);
    } else {
        snprintf(response, MAX_MSG, "ERROR: Unknown command or missing parameters\n");
    }
    
    send(client_sock, response, strlen(response), 0);
}

int main() {
    int client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    signal(SIGINT, cleanup_handler);
    signal(SIGTERM, cleanup_handler);
    
    if (init_db() != 0) {
        return 1;
    }
    
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket");
        return 1;
    }
    
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_sock);
        return 1;
    }
    
    if (listen(server_sock, 5) < 0) {
        perror("listen");
        close(server_sock);
        return 1;
    }
    
    printf("Dupsort server listening on port %d\n", PORT);
    printf("Commands: ADD, GET, COUNT, DEL, DELVAL\n");
    printf("Press Ctrl+C to stop\n");
    
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            if (errno == EINTR) break;
            perror("accept");
            continue;
        }
        
        printf("Client connected from %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));
        
        handle_client(client_sock);
        close(client_sock);
    }
    
    cleanup_handler(0);
    return 0;
}