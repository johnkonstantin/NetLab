#include "send.h"

void* senderThread(void* arg) {
    char msg[MSG_BUF_SIZE];
    snprintf(msg, MSG_BUF_SIZE, "%s %s", uuidStr, localAddr);

    int sock;
    void* addr;
    int addrLen;
    if (version == IPV4) {
        addr = malloc(sizeof(struct sockaddr_in));
        addrLen = sizeof(struct sockaddr_in);
    }
    else {
        addr = malloc(sizeof(struct sockaddr_in6));
        addrLen = sizeof(struct sockaddr_in6);
    }
    getSendSocketAndAddr(version, multicastAddr, port, &sock, addr);

    while (1) {
        sendto(sock, msg, strlen(msg) + 1, 0, (const struct sockaddr*) addr, addrLen);
        usleep(SEND_INTERVAL_MS * 1000);
    }

    pthread_exit(NULL);
}
