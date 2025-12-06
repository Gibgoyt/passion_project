/* Simple HTTP server with async client using ONLY uSockets public API */
#include <libusockets.h>
const int SSL = 1;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_RESPONSE_SIZE 8192
#define MAX_PENDING_REQUESTS 100

/* Pending request structure */
struct pending_request {
    int request_id;
    struct us_socket_t *server_socket;
    struct pending_request *next;
};

/* HTTP server socket extension */
struct http_server_socket {
    int request_id;
    int waiting_for_response;
};

/* HTTP client socket extension */
struct http_client_socket {
    int request_id;
    char response_buffer[MAX_RESPONSE_SIZE];
    int response_length;
    struct us_socket_t *server_socket;
};

/* Global state - single threaded, all in main loop */
static struct us_loop_t *main_loop;
static struct us_socket_context_t *client_context;
static struct pending_request *pending_head = NULL;
static int next_request_id = 1;

/* HTTP request template */
const char *http_request = 
    "GET /posts/1 HTTP/1.1\r\n"
    "Host: jsonplaceholder.typicode.com\r\n"
    "User-Agent: uSockets-Client/1.0\r\n"
    "Accept: application/json\r\n"
    "Connection: close\r\n"
    "\r\n";

/* Find end of HTTP headers */
static char* find_headers_end(char *data, int length) {
    for (int i = 0; i < length - 3; i++) {
        if (data[i] == '\r' && data[i+1] == '\n' && 
            data[i+2] == '\r' && data[i+3] == '\n') {
            return &data[i + 4];
        }
    }
    return NULL;
}

/* Add pending request */
static void add_pending_request(int request_id, struct us_socket_t *server_socket) {
    struct pending_request *req = malloc(sizeof(struct pending_request));
    req->request_id = request_id;
    req->server_socket = server_socket;
    req->next = pending_head;
    pending_head = req;
}

/* Find and remove pending request */
static struct us_socket_t* find_pending_request(int request_id) {
    struct pending_request **current = &pending_head;
    
    while (*current) {
        if ((*current)->request_id == request_id) {
            struct pending_request *found = *current;
            struct us_socket_t *server_socket = found->server_socket;
            *current = found->next;
            free(found);
            return server_socket;
        }
        current = &(*current)->next;
    }
    return NULL;
}

/* Client socket handlers */
struct us_socket_t *on_client_open(struct us_socket_t *s, int is_client, char *ip, int ip_length) {
    printf("Connected to API server, sending request\n");
    us_socket_write(SSL, s, http_request, strlen(http_request), 0);
    return s;
}

struct us_socket_t *on_client_data(struct us_socket_t *s, char *data, int length) {
    struct http_client_socket *client_socket = (struct http_client_socket *) us_socket_ext(SSL, s);
    
    /* Append data to response buffer */
    int space_left = MAX_RESPONSE_SIZE - client_socket->response_length - 1;
    int copy_length = (length < space_left) ? length : space_left;
    
    if (copy_length > 0) {
        memcpy(client_socket->response_buffer + client_socket->response_length, data, copy_length);
        client_socket->response_length += copy_length;
        client_socket->response_buffer[client_socket->response_length] = '\0';
    }
    
    printf("Received %d bytes from API\n", length);
    return s;
}

struct us_socket_t *on_client_close(struct us_socket_t *s, int code, void *reason) {
    struct http_client_socket *client_socket = (struct http_client_socket *) us_socket_ext(SSL, s);
    
    printf("API connection closed, processing response\n");
    
    /* Find the server socket waiting for this response */
    struct us_socket_t *server_socket = find_pending_request(client_socket->request_id);
    
    if (server_socket) {
        /* Find the JSON body after headers */
        char *body_start = find_headers_end(client_socket->response_buffer, client_socket->response_length);
        
        char response[MAX_RESPONSE_SIZE];
        int response_length;
        
        if (body_start) {
            int body_length = client_socket->response_length - (body_start - client_socket->response_buffer);
            response_length = snprintf(response, MAX_RESPONSE_SIZE,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %d\r\n"
                "Connection: keep-alive\r\n"
                "\r\n%.*s", body_length, body_length, body_start);
        } else {
            response_length = snprintf(response, MAX_RESPONSE_SIZE,
                "HTTP/1.1 500 Internal Server Error\r\n"
                "Content-Length: 27\r\n"
                "\r\n{\"error\":\"Invalid response\"}");
        }
        
        /* Send response back to client */
        us_socket_write(SSL, server_socket, response, response_length, 0);
        
        /* Mark server socket as no longer waiting */
        struct http_server_socket *server_ext = (struct http_server_socket *)us_socket_ext(SSL, server_socket);
        server_ext->waiting_for_response = 0;
        
        printf("Sent response back to client\n");
    } else {
        printf("Warning: No pending request found for ID %d\n", client_socket->request_id);
    }
    
    return s;
}

struct us_socket_t *on_client_end(struct us_socket_t *s) {
    return us_socket_close(SSL, s, 0, NULL);
}

struct us_socket_t *on_client_connect_error(struct us_socket_t *s, int code) {
    struct http_client_socket *client_socket = (struct http_client_socket *) us_socket_ext(SSL, s);
    
    printf("Failed to connect to API server\n");
    
    /* Find the server socket waiting for this response */
    struct us_socket_t *server_socket = find_pending_request(client_socket->request_id);
    
    if (server_socket) {
        char error_response[] = 
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Length: 29\r\n"
            "\r\n{\"error\":\"Connection failed\"}";
        
        us_socket_write(SSL, server_socket, error_response, strlen(error_response), 0);
        
        struct http_server_socket *server_ext = (struct http_server_socket *)us_socket_ext(SSL, server_socket);
        server_ext->waiting_for_response = 0;
    }
    
    return s;
}

/* Server event handlers */
void on_wakeup(struct us_loop_t *loop) {
    /* Nothing needed here for our simple approach */
}

void on_pre(struct us_loop_t *loop) {
    /* Nothing needed */
}

void on_post(struct us_loop_t *loop) {
    /* Nothing needed */
}

struct us_socket_t *on_server_open(struct us_socket_t *s, int is_client, char *ip, int ip_length) {
    struct http_server_socket *server_socket = (struct http_server_socket *)us_socket_ext(SSL, s);
    server_socket->request_id = 0;
    server_socket->waiting_for_response = 0;
    
    us_socket_timeout(SSL, s, 30);
    printf("Client connected to server\n");
    
    return s;
}

struct us_socket_t *on_server_data(struct us_socket_t *s, char *data, int length) {
    struct http_server_socket *server_socket = (struct http_server_socket *)us_socket_ext(SSL, s);
    
    /* Check if this is a GET / request and we're not already waiting */
    if (strncmp(data, "GET /", 5) == 0 && !server_socket->waiting_for_response) {
        printf("Received GET / request, making API call\n");
        
        server_socket->request_id = next_request_id++;
        server_socket->waiting_for_response = 1;
        
        /* Add to pending requests */
        add_pending_request(server_socket->request_id, s);
        
        /* Create HTTP client connection to API */
        struct us_socket_t *client_socket = us_socket_context_connect(SSL, client_context, 
            "jsonplaceholder.typicode.com", 443, NULL, 0, sizeof(struct http_client_socket));
        
        if (client_socket) {
            struct http_client_socket *client_ext = (struct http_client_socket *)us_socket_ext(SSL, client_socket);
            client_ext->request_id = server_socket->request_id;
            client_ext->response_length = 0;
            memset(client_ext->response_buffer, 0, MAX_RESPONSE_SIZE);
            
            printf("Initiated API connection for request %d\n", server_socket->request_id);
        } else {
            printf("Failed to create API connection\n");
            
            char error_response[] = 
                "HTTP/1.1 500 Internal Server Error\r\n"
                "Content-Length: 28\r\n"
                "\r\n{\"error\":\"Cannot connect\"}";
            
            us_socket_write(SSL, s, error_response, strlen(error_response), 0);
            server_socket->waiting_for_response = 0;
            
            /* Remove from pending */
            find_pending_request(server_socket->request_id);
        }
    }
    
    us_socket_timeout(SSL, s, 30);
    return s;
}

struct us_socket_t *on_server_close(struct us_socket_t *s, int code, void *reason) {
    printf("Client disconnected from server\n");
    return s;
}

struct us_socket_t *on_server_timeout(struct us_socket_t *s) {
    return us_socket_close(SSL, s, 0, NULL);
}

struct us_socket_t *on_server_end(struct us_socket_t *s) {
    us_socket_shutdown(SSL, s);
    return us_socket_close(SSL, s, 0, NULL);
}

int main() {
    printf("Starting simple uSockets HTTP server with async API client\n");
    
    /* Create main event loop */
    main_loop = us_create_loop(0, on_wakeup, on_pre, on_post, 0);
    
    /* Create SSL client context for API calls */
    struct us_socket_context_options_t client_options = {};
    client_context = us_create_socket_context(SSL, main_loop, sizeof(struct http_client_socket), client_options);
    
    if (!client_context) {
        printf("Failed to create SSL client context\n");
        return 1;
    }
    
    /* Set up client event handlers */
    us_socket_context_on_open(SSL, client_context, on_client_open);
    us_socket_context_on_data(SSL, client_context, on_client_data);
    us_socket_context_on_close(SSL, client_context, on_client_close);
    us_socket_context_on_end(SSL, client_context, on_client_end);
    us_socket_context_on_connect_error(SSL, client_context, on_client_connect_error);
    
    /* Create server context */
    struct us_socket_context_options_t server_options = {};
    struct us_socket_context_t *server_context = us_create_socket_context(SSL, main_loop, 
                                                                          sizeof(struct http_server_socket), server_options);
    
    if (!server_context) {
        printf("Failed to create server context\n");
        return 1;
    }
    
    /* Set up server event handlers */
    us_socket_context_on_open(SSL, server_context, on_server_open);
    us_socket_context_on_data(SSL, server_context, on_server_data);
    us_socket_context_on_close(SSL, server_context, on_server_close);
    us_socket_context_on_timeout(SSL, server_context, on_server_timeout);
    us_socket_context_on_end(SSL, server_context, on_server_end);
    
    /* Start listening */
    struct us_listen_socket_t *listen_socket = us_socket_context_listen(SSL, server_context, 0, 3000, 0, 
                                                                        sizeof(struct http_server_socket));
    
    if (listen_socket) {
        printf("Server listening on port 3000\n");
        printf("Try: curl http://localhost:3000/\n");
        us_loop_run(main_loop);
    } else {
        printf("Failed to listen on port 3000\n");
        return 1;
    }
    
    /* Cleanup */
    us_socket_context_free(SSL, client_context);
    us_socket_context_free(SSL, server_context);
    us_loop_free(main_loop);
    
    return 0;
}
