#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 9999
#define MAX_MSG 1024

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[MAX_MSG];
    char command[MAX_MSG];
    
    if (argc < 2) {
        printf("Usage:\n");
        printf("  %s GET <key>\n", argv[0]);
        printf("  %s SET <key> <value>\n", argv[0]);
        printf("  %s DEL <key>\n", argv[0]);
        printf("\nExamples:\n");
        printf("  %s SET name \"John Doe\"\n", argv[0]);
        printf("  %s GET name\n", argv[0]);
        printf("  %s DEL name\n", argv[0]);
        return 1;
    }
    
    command[0] = '\0';
    for (int i = 1; i < argc; i++) {
        strcat(command, argv[i]);
        if (i < argc - 1) strcat(command, " ");
    }
    strcat(command, "\n");
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect - make sure server is running");
        close(sock);
        return 1;
    }
    
    send(sock, command, strlen(command), 0);
    
    int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
    }
    
    close(sock);
    return 0;
}