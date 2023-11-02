#ifndef LAB1_SEND_H
#define LAB1_SEND_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <uuid/uuid.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <net/if.h>
#include "constants.h"
#include "socketUtils.h"
#include "aliveList.h"


extern char* multicastAddr;
extern int port;
extern char uuidStr[UUID_STR_LEN];
extern char localAddr[IPV6_ADDR_MAX_STR_LEN];
extern IP_VERSION version;

void* senderThread(void* arg);

#endif //LAB1_SEND_H
