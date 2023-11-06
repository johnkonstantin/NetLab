#ifndef LAB2_SOCKETUTILS_H
#define LAB2_SOCKETUTILS_H

#include <arpa/inet.h>
#include <unistd.h>

int getlistetingSocket(int port, int wait_size);
int getClientSocket(struct in_addr server_addr, int port);
ssize_t sendData(int sock, void* data, size_t size);
ssize_t recvData(int sock, void* data, size_t size);

#endif //LAB2_SOCKETUTILS_H
