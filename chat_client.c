#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 6000

int sockid;  // Global socket descriptor for the client

void* receive_message(void* arg) {
    char msg[1000];
    struct sockaddr_in servaddr;
    socklen_t servlen = sizeof(struct sockaddr_in);

    while (1) {
        bzero(msg, sizeof(msg));
        int recvid = recvfrom(sockid, msg, sizeof(msg), 0, (struct sockaddr*)&servaddr, &servlen);
        if (recvid < 0) {
            printf("Error receiving message.\n");
            break;
        }
        printf("Server: %s\n", msg);

        if (strncmp("bye", msg, 3) == 0) {
            printf("Server ended the chat.\n");
            break;
        }
    }
    return NULL;
}

void* send_message(void* arg) {
    char msg[1000];
    struct sockaddr_in servaddr;
    socklen_t servlen = sizeof(struct sockaddr_in);

    while (1) {
        bzero(msg, sizeof(msg));
        printf("Client: ");
        fgets(msg, sizeof(msg), stdin);

        int sendid = sendto(sockid, msg, sizeof(msg), 0, (struct sockaddr*)&servaddr, sizeof(struct sockaddr_in));
        if (sendid < 0) {
            printf("Error sending message.\n");
            break;
        }

        if (strncmp("bye", msg, 3) == 0) {
            printf("You ended the chat.\n");
            break;
        }
    }
    return NULL;
}

int main() {
    struct sockaddr_in servaddr;
    pthread_t recv_thread, send_thread;

    // Creating socket
    sockid = socket(AF_INET, SOCK_STREAM, 0);
    if (sockid < 0) {
        printf("Error creating socket.\n");
        exit(1);
    }

    // Configure server address
    bzero((char*)&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Server IP
    servaddr.sin_port = htons(PORT);

    // Connecting to server
    int connectid = connect(sockid, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (connectid < 0) {
        printf("Error connecting to server.\n");
        close(sockid);
        exit(1);
    }
    printf("Connected to server.\n");

    // Create threads for sending and receiving messages
    pthread_create(&recv_thread, NULL, receive_message, NULL);
    pthread_create(&send_thread, NULL, send_message, NULL);

    // Wait for threads to finish
    pthread_join(recv_thread, NULL);
    pthread_join(send_thread, NULL);

    close(sockid);
    return 0;
}
