#ifndef LAB1_ALIVELIST_H
#define LAB1_ALIVELIST_H

#include <string.h>
#include <malloc.h>
#include <uuid/uuid.h>
#include <net/if.h>
#include "constants.h"

typedef struct Node_t Node;
struct Node_t {
    char IPADDR[IPV6_ADDR_MAX_STR_LEN];
    char UUID[UUID_STR_LEN];
};

ssize_t addNode(Node* head, size_t len, size_t size, const char* IP, const char* UUID);

#endif //LAB1_ALIVELIST_H
