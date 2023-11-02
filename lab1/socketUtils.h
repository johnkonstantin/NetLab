#ifndef LAB1_SOCKETUTILS_H
#define LAB1_SOCKETUTILS_H

#include <sys/socket.h>
#include <stddef.h>
#include <string.h>
#include <arpa/inet.h>
#include <malloc.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include "constants.h"

typedef enum IP_VERSION_e IP_VERSION;
enum IP_VERSION_e {
    IPV4,
    IPV6,
    ERROR
};

IP_VERSION getIPType(const char* ip);
int checkIPInMulticast(const char* ipAddr);
int getSendSocketAndAddr(IP_VERSION ipVersion, const char* ipStr, int port, int* sock, void* addr);
int getReceiveSocketAndAddr(IP_VERSION ipVersion, const char* ipStr, int port, int* sock, void* addr);
int getLocalIPAddr(IP_VERSION ipVersion, int sock, const char* interface, char* ipAddr);

#endif //LAB1_SOCKETUTILS_H
