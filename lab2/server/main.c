#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "clientHandler.h"
#include "../socketUtils/socketUtils.h"

#define WAIT_SIZE 16

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Type port!\n");
        return 0;
    }
    int port = atoi(argv[1]);
    if (port <= 0) {
        printf("Wrong port!\n");
        return 0;
    }

    int listen_sock = getlistetingSocket(port, WAIT_SIZE);
    if (listen_sock < 0) {
        fprintf(stderr, "Could not create listening socket!\n");
        return 1;
    }

    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    int client_sock;

    while (1) {
        if ((client_sock = accept(listen_sock, (struct sockaddr*) &client_addr, &client_addr_len)) < 0) {
            fprintf(stderr, "Could not open client socket!\n");
            continue;
        }

        int* args = malloc(sizeof(int));
        *args = client_sock;

        pthread_t thread;
        int err = pthread_create(&thread, NULL, clientHandler, args);
        if (err != 0) {
            fprintf(stderr, "Could not create thread!\n");
            free(args);
        }
    }
    return 0;
}