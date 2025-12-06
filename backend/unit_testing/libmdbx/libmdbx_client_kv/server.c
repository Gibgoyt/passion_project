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
    
    rc = mdbx_env_open(env, DB_PATH, MDBX_NOSUBDIR | MDBX_COALESCE | MDBX_LIFORECLAIM, 0664);
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
    
    rc = mdbx_dbi_open(txn, NULL, MDBX_CREATE, &dbi);
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
    
    printf("Database initialized at %s\n", DB_PATH);
    return 0;
}

int handle_get(const char *key, char *response) {
    MDBX_txn *txn;
    MDBX_val k, v;
    int rc;
    
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_RDONLY, &txn);
    if (rc != MDBX_SUCCESS) {
        snprintf(response, MAX_MSG, "ERROR: txn_begin failed\n");
        return -1;
    }
    
    k.iov_base = (void*)key;
    k.iov_len = strlen(key);
    
    rc = mdbx_get(txn, dbi, &k, &v);
    if (rc == MDBX_SUCCESS) {
        snprintf(response, MAX_MSG, "OK: %.*s\n", (int)v.iov_len, (char*)v.iov_base);
    } else if (rc == MDBX_NOTFOUND) {
        snprintf(response, MAX_MSG, "ERROR: Key not found\n");
    } else {
        snprintf(response, MAX_MSG, "ERROR: %s\n", mdbx_strerror(rc));
    }
    
    mdbx_txn_abort(txn);
    return rc == MDBX_SUCCESS ? 0 : -1;
}

int handle_set(const char *key, const char *value, char *response) {
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
    
    snprintf(response, MAX_MSG, "OK: Set %s = %s\n", key, value);
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
    } else if (strcmp(cmd, "GET") == 0) {
        handle_get(key, response);
    } else if (strcmp(cmd, "SET") == 0 && parsed == 3) {
        handle_set(key, value, response);
    } else if (strcmp(cmd, "DEL") == 0) {
        handle_del(key, response);
    } else {
        snprintf(response, MAX_MSG, "ERROR: Unknown command\n");
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
    
    printf("Server listening on port %d\n", PORT);
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