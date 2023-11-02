#include "aliveList.h"

ssize_t addNode(Node *head, size_t len, size_t size, const char *IP, const char *UUID) {
    int flag = 1;
    for (size_t i = 0; i < len; ++i) {
        uuid_t u1;
        uuid_t u2;
        uuid_parse(head[i].UUID, u1);
        uuid_parse(UUID, u2);
        if (uuid_compare(u1, u2) == 0) {
            flag = 0;
            break;
        }
    }
    if (flag == 1) {
        if (len < size) {
            strcpy(head[len].IPADDR, IP);
            strcpy(head[len].UUID, UUID);
            ++len;
        }
        else {
            return -1;
        }
    }
    return len;
}