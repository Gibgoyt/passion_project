#include <libusockets.h>
#include <stdio.h>
#include <stdlib.h>

const int SSL = 1;

/* Socket extension - tracks response streaming offset */
struct http_socket {
    int offset;
};

/* Context extension - holds shared response data */
struct http_context {
    char *response;
    int length;
};

/* Forward declarations */
struct us_socket_t *on_http_socket_open(struct us_socket_t *s, int is_client, char *ip, int ip_length);
struct us_socket_t *on_http_socket_data(struct us_socket_t *s, char *data, int length);
struct us_socket_t *on_http_socket_writable(struct us_socket_t *s);
struct us_socket_t *on_http_socket_close(struct us_socket_t *s, int code, void *reason);
struct us_socket_t *on_http_socket_timeout(struct us_socket_t *s);
struct us_socket_t *on_http_socket_end(struct us_socket_t *s);

/* Loop event handlers (required) */
void on_wakeup(struct us_loop_t *loop) {
    (void)loop; /* Suppress unused parameter warning */
}

void on_pre(struct us_loop_t *loop) {
    (void)loop; /* Suppress unused parameter warning */
}

void on_post(struct us_loop_t *loop) {
    (void)loop; /* Suppress unused parameter warning */
}

/* On connection open - reset offset for streaming */
struct us_socket_t *on_http_socket_open(struct us_socket_t *s, int is_client, char *ip, int ip_length) {
    (void)is_client; /* Suppress unused parameter warning */

    struct http_socket *socket_ext = (struct http_socket *) us_socket_ext(SSL, s);
    socket_ext->offset = 0;

    printf("Client connected from IP: %.*s\n", ip_length, ip);

    /* Set idle timeout to 30 seconds */
    us_socket_timeout(SSL, s, 30);

    return s;
}

/* On data received - parse HTTP request and send response */
struct us_socket_t *on_http_socket_data(struct us_socket_t *s, char *data, int length) {
    struct http_socket *socket_ext = (struct http_socket *) us_socket_ext(SSL, s);

    /* Reset timeout on activity */
    us_socket_timeout(SSL, s, 30);

    /* Check if this looks like an HTTP request */
    if (length > 4 && (data[0] == 'G' || data[0] == 'P' || data[0] == 'H')) {
        printf("Received HTTP request: %.*s\n", length > 50 ? 50 : length, data);

        /* Reset streaming offset for new request */
        socket_ext->offset = 0;

        /* Try to send the response immediately */
        return on_http_socket_writable(s);
    }

    return s;
}

/* On socket writable - stream response data */
struct us_socket_t *on_http_socket_writable(struct us_socket_t *s) {
    struct http_context *context_ext = (struct http_context *) us_socket_context_ext(SSL, us_socket_context(SSL, s));
    struct http_socket *socket_ext = (struct http_socket *) us_socket_ext(SSL, s);

    /* Continue streaming from current offset */
    if (socket_ext->offset < context_ext->length) {
        int sent = us_socket_write(SSL, s, context_ext->response + socket_ext->offset,
                                  context_ext->length - socket_ext->offset, 0);
        if (sent > 0) {
            socket_ext->offset += sent;
        }

        /* If not fully sent, we'll get called again when writable */
        if (socket_ext->offset < context_ext->length) {
            return s;
        }
    }

    /* Response fully sent - keep connection alive for HTTP/1.1 */
    printf("Response sent, keeping connection alive\n");
    return s;
}

/* On connection close */
struct us_socket_t *on_http_socket_close(struct us_socket_t *s, int code, void *reason) {
    (void)reason; /* Suppress unused parameter warning */

    printf("Client disconnected with code: %d\n", code);
    return s;
}

/* On connection timeout */
struct us_socket_t *on_http_socket_timeout(struct us_socket_t *s) {
    printf("Client connection timed out after 30 seconds\n");
    return us_socket_close(SSL, s, 0, NULL);
}

/* On connection end */
struct us_socket_t *on_http_socket_end(struct us_socket_t *s) {
    printf("Client connection ended gracefully\n");
    return us_socket_close(SSL, s, 0, NULL);
}

int main() {
    /* Create the event loop */
    struct us_loop_t *loop = us_create_loop(0, on_wakeup, on_pre, on_post, 0);

    /* Configure SSL options with local test certificates */
    struct us_socket_context_options_t options = {};
    options.key_file_name = "../../../unit_testing/http11_server/certs/server.key";
    options.cert_file_name = "../../../unit_testing/http11_server/certs/server.crt";
    options.passphrase = "";

    /* Create SSL socket context */
    struct us_socket_context_t *http_context = us_create_socket_context(SSL, loop,
                                                sizeof(struct http_context), options);

    if (!http_context) {
        printf("ERROR: Could not load SSL certificates from ../../../unit_testing/http11_server/certs/\n");
        printf("Please ensure server.crt and server.key exist and are readable\n");
        exit(1);
    }

    /* Build HTTP/1.1 response with keep-alive */
    struct http_context *context_ext = (struct http_context *) us_socket_context_ext(SSL, http_context);

    const char body[] = "<html><head><title>uSockets HTTP/1.1 SSL Test Server</title></head>"
                       "<body><h1>Hello from SSL HTTPS Server!</h1>"
                       "<p>This is a uSockets HTTP/1.1 SSL test server running on port 2053.</p>"
                       "<p>Connection established successfully with SSL/TLS encryption.</p>"
                       "</body></html>";

    static char response[2048];
    context_ext->length = snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %ld\r\n"
        "Connection: keep-alive\r\n"
        "Server: uSockets-SSL/1.0\r\n"
        "\r\n%s",
        sizeof(body) - 1, body);

    context_ext->response = response;

    /* Register socket event handlers */
    us_socket_context_on_open(SSL, http_context, on_http_socket_open);
    us_socket_context_on_data(SSL, http_context, on_http_socket_data);
    us_socket_context_on_writable(SSL, http_context, on_http_socket_writable);
    us_socket_context_on_close(SSL, http_context, on_http_socket_close);
    us_socket_context_on_timeout(SSL, http_context, on_http_socket_timeout);
    us_socket_context_on_end(SSL, http_context, on_http_socket_end);

    /* Start listening on port 2053 */
    struct us_listen_socket_t *listen_socket = us_socket_context_listen(SSL, http_context, 0, 2053, 0,
                                                                       sizeof(struct http_socket));

    if (listen_socket) {
        printf("SSL HTTPS server listening on https://localhost:2053\n");
        printf("Using SSL certificates from ../../../unit_testing/http11_server/certs/\n");
        printf("Press Ctrl+C to stop the server\n");

        /* Run the event loop */
        us_loop_run(loop);
    } else {
        printf("ERROR: Failed to listen on port 2053\n");
        exit(1);
    }

    return 0;
}