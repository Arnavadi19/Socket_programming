#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 6000

int newsockid;  // Global socket descriptor for the new connection

void* receive_message(void* arg) {
    char msg[1000];
    struct sockaddr_in client;
    socklen_t clientlen = sizeof(struct sockaddr_in);

    while (1) {
        bzero(msg, sizeof(msg));
        int recvid = recvfrom(newsockid, msg, sizeof(msg), 0, (struct sockaddr*)&client, &clientlen);
        if (recvid < 0) {
            printf("Error receiving message.\n");
            break;
        }
        printf("Client: %s\n", msg);

        if (strncmp("bye", msg, 3) == 0) {
            printf("Client ended the chat.\n");
            break;
        }
    }
    return NULL;
}

void* send_message(void* arg) {
    char msg[1000];
    struct sockaddr_in client;
    socklen_t clientlen = sizeof(struct sockaddr_in);

    while (1) {
        bzero(msg, sizeof(msg));
        printf("Server: ");
        fgets(msg, sizeof(msg), stdin);

        int sendid = sendto(newsockid, msg, sizeof(msg), 0, (struct sockaddr*)&client, sizeof(struct sockaddr_in));
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
    int sockid, bindid;
    struct sockaddr_in myaddr, client;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    pthread_t recv_thread, send_thread;

    // Creating socket
    sockid = socket(AF_INET, SOCK_STREAM, 0);
    if (sockid < 0) {
        printf("Error creating socket.\n");
        exit(1);
    }

    // Configure server address
    bzero((char*)&myaddr, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(PORT);

    // Binding socket
    bindid = bind(sockid, (struct sockaddr*)&myaddr, sizeof(myaddr));
    if (bindid < 0) {
        printf("Error binding socket.\n");
        close(sockid);
        exit(1);
    }

    // Listening for connection
    listen(sockid, 5);
    printf("Server listening on port %d...\n", PORT);

    // Accepting connection from client
    newsockid = accept(sockid, (struct sockaddr*)&client, &clientlen);
    if (newsockid < 0) {
        printf("Error accepting connection.\n");
        close(sockid);
        exit(1);
    }
    printf("Client connected.\n");

    // Create threads for sending and receiving messages
    pthread_create(&recv_thread, NULL, receive_message, NULL);
    pthread_create(&send_thread, NULL, send_message, NULL);

    // Wait for threads to finish
    pthread_join(recv_thread, NULL);
    pthread_join(send_thread, NULL);

    close(newsockid);
    close(sockid);
    return 0;
}
