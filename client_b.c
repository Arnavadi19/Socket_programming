// client_b.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8000
#define MAX_BUFFER 1024
#define WINDOW_SIZE 5
#define MAX_MESSAGES 10

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER];
    int window_start = 0;
    int next_to_send = 0;
    int acks_received = 0;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET; //TCP
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(1);
    }
    printf("Connected to server\n");

    while (acks_received < MAX_MESSAGES) {
        // Send messages within window
        while (next_to_send < window_start + WINDOW_SIZE && next_to_send < MAX_MESSAGES) {
            sprintf(buffer, "Hello %d", next_to_send);
            send(client_socket, buffer, strlen(buffer), 0);
            printf("Sent: %s\n", buffer);
            next_to_send++;
        }

        // Wait for ACK
        memset(buffer, '\0', sizeof(buffer));
        recv(client_socket, buffer, sizeof(buffer), 0);
        
        if (strcmp(buffer, "ACK") == 0) {
            printf("Received ACK for message %d\n", window_start);
            window_start++;
            acks_received++;
        }
    }

    // Send exit message
    strcpy(buffer, "exit");
    send(client_socket, buffer, strlen(buffer), 0);

    close(client_socket);
    return 0;
}