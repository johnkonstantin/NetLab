#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <uuid/uuid.h>
#include "constants.h"
#include "aliveList.h"
#include "socketUtils.h"
#include "print.h"
#include "send.h"

pthread_mutex_t mutex;
char* multicastAddr;
int port;
uuid_t uuid;
char uuidStr[UUID_STR_LEN];
Node* head;
size_t aliveListLen;
char localAddr[IPV6_ADDR_MAX_STR_LEN];
IP_VERSION version;
void* addr;

int main(int argc, char** argv) {
    if (argc != 4) {
        printf("Command line args should be multicast group addr, port and interface!\n");
        return 0;
    }

    int err;

    multicastAddr = argv[1];
    version = getIPType(multicastAddr);

    if (version == ERROR || !checkIPInMulticast(multicastAddr)) {

        fprintf(stderr, "Invalid address!\n");
        return 1;
    }

    port = atoi(argv[2]);
    char* interface = argv[3];

    int sock;
    int addrLen = (version == IPV4) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
    addr = malloc(addrLen);
    err = getReceiveSocketAndAddr(version, multicastAddr, port, &sock, addr);
    if (err != 0) {
        fprintf(stderr, "Error create socket!\n");
        return 1;
    }
    err = getLocalIPAddr(version, sock, interface, localAddr);
    if (err != 0) {
        fprintf(stderr, "Error get local IP!\n");
        return 1;
    }

    char inMsg[MSG_BUF_SIZE];

    uuid_generate_random(uuid);
    uuid_unparse(uuid, uuidStr);

    pthread_t sender;
    err = pthread_create(&sender, NULL, senderThread, NULL);
    if (err != 0) {
        fprintf(stderr, "Pthread_create error: %s\n", strerror(err));
        return 1;
    }

    head = malloc(ALIVE_LIST_SIZE * sizeof(Node));
    aliveListLen = 0;

    pthread_mutex_init(&mutex, NULL);

    pthread_t printer;
    err = pthread_create(&printer, NULL, printerThread, NULL);
    if (err != 0) {
        fprintf(stderr, "Pthread_create error: %s\n", strerror(err));
        return 1;
    }

    int nbytes;
    char uuidR[37];
    uuidR[36] = 0;
    char IP[IPV6_ADDR_MAX_STR_LEN];
    while (1) {
        nbytes = recvfrom(sock, inMsg, MSG_BUF_SIZE, 0, (struct sockaddr*) addr, &addrLen);

        if (nbytes > 0) {
            inMsg[nbytes] = 0;
            memcpy(uuidR, inMsg, 36);
            strcpy(IP, inMsg + 37);
            pthread_mutex_lock(&mutex);
            aliveListLen = addNode(head, aliveListLen, ALIVE_LIST_SIZE, IP, uuidR);
            pthread_mutex_unlock(&mutex);
        }
        if (nbytes < 0) {
            fprintf(stderr, "Error recvfrom!\n");
            return 1;
        }
    }

    return 0;
}