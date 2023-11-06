#ifndef LAB2_DNS_H
#define LAB2_DNS_H

#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

struct in_addr* dns_resolve(const char* name);

#endif //LAB2_DNS_H
