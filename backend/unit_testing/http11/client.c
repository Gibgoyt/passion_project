#include <libusockets.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int SSL = 1;

/* Global configuration */
static int total_connections = 1;
static int connections_remaining = 0;
static char *target_host = NULL;
static int target_port = 2053;
static int responses = 0;
static int connections_established = 0;
static int benchmark_started = 0;

/* Socket extension - per-connection state */
struct http_client_socket {
    int connected;
    int request_sent;
    int response_received;
    char request_buffer[512];  /* Safe local storage for request */
    int request_length;
};

/* Context extension - shared global state */
struct http_client_context {
    int total_connections_made;
    int total_responses;
};

/* Forward declarations */
struct us_socket_t *on_client_socket_open(struct us_socket_t *s, int is_client, char *ip, int ip_length);
struct us_socket_t *on_client_socket_data(struct us_socket_t *s, char *data, int length);
struct us_socket_t *on_client_socket_writable(struct us_socket_t *s);
struct us_socket_t *on_client_socket_close(struct us_socket_t *s, int code, void *reason);
struct us_socket_t *on_client_socket_timeout(struct us_socket_t *s);
struct us_socket_t *on_client_socket_end(struct us_socket_t *s);
struct us_socket_t *on_client_socket_long_timeout(struct us_socket_t *s);
struct us_socket_t *on_client_socket_connect_error(struct us_socket_t *s, int code);

/* Loop event handlers */
void on_wakeup(struct us_loop_t *loop) {
    (void)loop;
}

void on_pre(struct us_loop_t *loop) {
    (void)loop;
}

void on_post(struct us_loop_t *loop) {
    (void)loop;
}

/* Host:port parsing function */
char *parse_host_port(const char *hostport, int *port) {
    char *colon = strchr(hostport, ':');
    if (!colon) {
        *port = 2053;  /* Default port */
        char *host = malloc(strlen(hostport) + 1);
        strcpy(host, hostport);
        return host;
    }

    int host_len = colon - hostport;
    char *host = malloc(host_len + 1);
    memcpy(host, hostport, host_len);
    host[host_len] = '\0';

    *port = atoi(colon + 1);
    return host;
}

/* On connection open - initialize and chain connections */
struct us_socket_t *on_client_socket_open(struct us_socket_t *s, int is_client, char *ip, int ip_length) {
    (void)is_client;
    (void)ip;
    (void)ip_length;

    struct http_client_socket *socket_ext = (struct http_client_socket *) us_socket_ext(SSL, s);
    struct http_client_context *context_ext = (struct http_client_context *) us_socket_context_ext(SSL, us_socket_context(SSL, s));

    /* Initialize ALL socket extension fields immediately */
    socket_ext->connected = 1;
    socket_ext->request_sent = 0;
    socket_ext->response_received = 0;
    socket_ext->request_length = 0;
    memset(socket_ext->request_buffer, 0, sizeof(socket_ext->request_buffer));

    connections_established++;
    context_ext->total_connections_made++;

    if (total_connections == 1) {
        printf("✅ SSL connection established to %s:%d\n", target_host, target_port);
    } else {
        printf("✅ Connection %d/%d established\n", connections_established, total_connections);
    }

    /* Prepare HTTP request in safe local buffer */
    socket_ext->request_length = snprintf(socket_ext->request_buffer, sizeof(socket_ext->request_buffer),
        "GET / HTTP/1.1\r\n"
        "Host: %s:%d\r\n"
        "User-Agent: uSockets-SSL-LoadTest/1.0\r\n"
        "Accept: */*\r\n"
        "Connection: close\r\n"
        "\r\n",
        target_host, target_port);

    /* Try to send request immediately */
    int sent = us_socket_write(SSL, s, socket_ext->request_buffer, socket_ext->request_length, 0);
    if (sent > 0) {
        socket_ext->request_sent = 1;
        if (total_connections == 1) {
            printf("📤 HTTP request sent immediately (%d bytes)\n", sent);
        }
    } else if (sent == 0) {
        /* SSL handshake not complete - will retry in on_writable */
        if (total_connections == 1) {
            printf("⏳ HTTP request pending (SSL handshake in progress)\n");
        }
    } else {
        printf("❌ Failed to send HTTP request (error: %d)\n", sent);
    }

    /* Chain next connection if needed */
    if (--connections_remaining > 0) {
        us_socket_context_connect(SSL, us_socket_context(SSL, s), target_host, target_port,
                                 NULL, 0, sizeof(struct http_client_socket));
    } else {
        /* All connections established - start benchmark */
        if (total_connections > 1) {
            printf("🚀 All %d connections established, starting benchmark...\n", total_connections);
        }
        benchmark_started = 1;
        us_socket_timeout(SSL, s, 8);  /* 8-second measurement intervals */
        us_socket_long_timeout(SSL, s, 1);  /* 1-minute progress reports */
    }

    return s;
}

/* On data received - count responses */
struct us_socket_t *on_client_socket_data(struct us_socket_t *s, char *data, int length) {
    struct http_client_socket *socket_ext = (struct http_client_socket *) us_socket_ext(SSL, s);
    struct http_client_context *context_ext = (struct http_client_context *) us_socket_context_ext(SSL, us_socket_context(SSL, s));

    socket_ext->response_received = 1;
    responses++;
    context_ext->total_responses++;

    if (total_connections == 1) {
        /* Single connection - show full response */
        printf("📥 Received HTTP response (%d bytes):\n", length);
        printf("=====================================\n");
        printf("%.*s", length > 500 ? 500 : length, data);  /* Limit output for large responses */
        if (length > 500) {
            printf("\n... (truncated)\n");
        }
        printf("=====================================\n");

        /* Parse response status */
        if (length >= 12 && strncmp(data, "HTTP/1.1 200", 12) == 0) {
            printf("✅ HTTP/1.1 200 OK response received\n");
        } else {
            printf("⚠️  Unexpected HTTP response status\n");
        }

        /* Check for expected content */
        if (strstr(data, "Hello from SSL HTTPS Server!") != NULL) {
            printf("✅ Expected server response content found\n");
        } else {
            printf("⚠️  Expected content not found in response\n");
        }
    }

    /* For load testing, close connection to make room for new ones */
    if (total_connections > 1) {
        return us_socket_close(SSL, s, 0, NULL);
    }

    return s;
}

/* On socket writable - retry pending writes after SSL handshake */
struct us_socket_t *on_client_socket_writable(struct us_socket_t *s) {
    struct http_client_socket *socket_ext = (struct http_client_socket *) us_socket_ext(SSL, s);

    /* Retry sending request if not yet sent */
    if (!socket_ext->request_sent && socket_ext->request_length > 0) {
        int sent = us_socket_write(SSL, s, socket_ext->request_buffer, socket_ext->request_length, 0);

        if (sent > 0) {
            socket_ext->request_sent = 1;
            if (total_connections == 1) {
                printf("📤 HTTP request sent after SSL handshake (%d bytes)\n", sent);
            }
        } else if (sent == 0) {
            /* Still pending - will try again */
            if (total_connections == 1) {
                printf("⏳ HTTP request still pending (will retry again)\n");
            }
        } else {
            printf("❌ Failed to send HTTP request on retry (error: %d)\n", sent);
        }
    }

    return s;
}

/* On connection close */
struct us_socket_t *on_client_socket_close(struct us_socket_t *s, int code, void *reason) {
    (void)reason;

    struct http_client_socket *socket_ext = (struct http_client_socket *) us_socket_ext(SSL, s);
    struct http_client_context *context_ext = (struct http_client_context *) us_socket_context_ext(SSL, us_socket_context(SSL, s));

    if (total_connections == 1) {
        /* Single connection test */
        printf("🔌 Connection closed (code: %d)\n", code);

        if (socket_ext->connected && socket_ext->request_sent && socket_ext->response_received) {
            printf("✅ SSL HTTPS test completed successfully!\n");
        } else {
            printf("❌ Test incomplete - Connected: %d, Request: %d, Response: %d\n",
                   socket_ext->connected, socket_ext->request_sent, socket_ext->response_received);
        }

        /* Print summary */
        printf("\n=== SSL HTTPS Test Summary ===\n");
        printf("Target: https://%s:%d\n", target_host, target_port);
        printf("Connections made: %d\n", context_ext->total_connections_made);
        printf("Responses received: %d\n", context_ext->total_responses);
        printf("=============================\n");

        exit(context_ext->total_responses > 0 ? 0 : 1);
    } else {
        /* Load testing - create new connection to maintain load */
        if (benchmark_started) {
            us_socket_context_connect(SSL, us_socket_context(SSL, s), target_host, target_port,
                                     NULL, 0, sizeof(struct http_client_socket));
        }
    }

    return s;
}

/* On connection timeout - metrics reporting */
struct us_socket_t *on_client_socket_timeout(struct us_socket_t *s) {
    if (benchmark_started && responses > 0) {
        printf("📊 Req/sec: %.2f (responses in last interval: %d)\n",
               (float)responses / 8.0, responses);  /* 8-second intervals */
        responses = 0;  /* Reset counter */
        us_socket_timeout(SSL, s, 8);  /* Schedule next measurement */
    } else if (total_connections == 1) {
        printf("❌ Connection timed out waiting for server response\n");
        return us_socket_close(SSL, s, 0, NULL);
    }

    return s;
}

/* On connection end */
struct us_socket_t *on_client_socket_end(struct us_socket_t *s) {
    if (total_connections == 1) {
        printf("🔚 Server ended connection gracefully\n");
    }
    return us_socket_close(SSL, s, 0, NULL);
}

/* Long timeout - minute progress reports */
struct us_socket_t *on_client_socket_long_timeout(struct us_socket_t *s) {
    if (benchmark_started) {
        struct http_client_context *context_ext = (struct http_client_context *) us_socket_context_ext(SSL, us_socket_context(SSL, s));
        printf("📈 === Minute mark === Total responses: %d, connections: %d\n",
               context_ext->total_responses, context_ext->total_connections_made);
        us_socket_long_timeout(SSL, s, 1);  /* Schedule next minute report */
    }
    return s;
}

/* On connection error - handle failed connection attempts */
struct us_socket_t *on_client_socket_connect_error(struct us_socket_t *s, int code) {
    printf("❌ Connection failed with error code: %d\n", code);

    if (total_connections == 1) {
        /* Single connection test - this is a fatal error */
        printf("❌ Failed to connect to %s:%d\n", target_host, target_port);
        printf("💡 Check if the server is running and the host/port are correct\n");
        exit(1);
    } else {
        /* Load testing - try to continue with remaining connections */
        if (connections_remaining > 0) {
            /* Still have connections to establish, try another one */
            us_socket_context_connect(SSL, us_socket_context(SSL, s), target_host, target_port,
                                     NULL, 0, sizeof(struct http_client_socket));
        }
        printf("⚠️  Connection failed, continuing with remaining connections...\n");
    }

    return s;
}

/* Print usage information */
void print_usage(const char *program_name) {
    printf("SSL HTTPS Load Testing Client\n");
    printf("Usage:\n");
    printf("  %s <host:port>                    - Single connection test\n", program_name);
    printf("  %s <connections> <host:port>      - Load test with multiple connections\n", program_name);
    printf("\nExamples:\n");
    printf("  %s localhost:2053\n", program_name);
    printf("  %s devbackend.splitdo.app:2053\n", program_name);
    printf("  %s 100 localhost:2053\n", program_name);
    printf("  %s 100 devbackend.splitdo.app:2053\n", program_name);
}

int main(int argc, char **argv) {
    /* Parse command line arguments */
    if (argc < 2 || argc > 3) {
        print_usage(argv[0]);
        return 1;
    }

    if (argc == 2) {
        /* Single connection: ./client host:port */
        total_connections = 1;
        target_host = parse_host_port(argv[1], &target_port);
    } else {
        /* Load testing: ./client connections host:port */
        total_connections = atoi(argv[1]);
        if (total_connections <= 0) {
            printf("❌ Error: Invalid number of connections: %s\n", argv[1]);
            return 1;
        }
        target_host = parse_host_port(argv[2], &target_port);
    }

    if (!target_host) {
        printf("❌ Error: Failed to parse host:port\n");
        return 1;
    }

    connections_remaining = total_connections;

    if (total_connections == 1) {
        printf("🚀 Starting SSL HTTPS client test\n");
        printf("Target: https://%s:%d\n", target_host, target_port);
        printf("==================================\n");
    } else {
        printf("🚀 Starting SSL HTTPS load test\n");
        printf("Target: https://%s:%d\n", target_host, target_port);
        printf("Connections: %d\n", total_connections);
        printf("==================================\n");
    }

    /* Create event loop */
    struct us_loop_t *loop = us_create_loop(0, on_wakeup, on_pre, on_post, 0);

    /* Configure SSL options for client (no client certificates needed) */
    struct us_socket_context_options_t options = {};

    /* Create SSL socket context */
    struct us_socket_context_t *client_context = us_create_socket_context(SSL, loop,
                                                sizeof(struct http_client_context), options);

    if (!client_context) {
        printf("❌ ERROR: Could not create SSL client context\n");
        free(target_host);
        return 1;
    }

    /* Initialize context extension */
    struct http_client_context *context_ext = (struct http_client_context *) us_socket_context_ext(SSL, client_context);
    context_ext->total_connections_made = 0;
    context_ext->total_responses = 0;

    /* Register socket event handlers */
    us_socket_context_on_open(SSL, client_context, on_client_socket_open);
    us_socket_context_on_data(SSL, client_context, on_client_socket_data);
    us_socket_context_on_writable(SSL, client_context, on_client_socket_writable);
    us_socket_context_on_close(SSL, client_context, on_client_socket_close);
    us_socket_context_on_timeout(SSL, client_context, on_client_socket_timeout);
    us_socket_context_on_end(SSL, client_context, on_client_socket_end);
    us_socket_context_on_long_timeout(SSL, client_context, on_client_socket_long_timeout);
    us_socket_context_on_connect_error(SSL, client_context, on_client_socket_connect_error);

    /* Start the first connection (others will chain from on_open) */
    struct us_socket_t *first_socket = us_socket_context_connect(SSL, client_context, target_host, target_port,
                                                               NULL, 0, sizeof(struct http_client_socket));

    if (!first_socket) {
        printf("❌ ERROR: Failed to initiate connection to %s:%d\n", target_host, target_port);
        free(target_host);
        return 1;
    }

    if (total_connections == 1) {
        printf("🔌 Connecting to SSL server...\n");
    } else {
        printf("🔌 Establishing %d SSL connections...\n", total_connections);
    }

    /* Run the event loop */
    us_loop_run(loop);

    /* Cleanup */
    free(target_host);
    return 0;
}