#include "dns.h"

struct in_addr* dns_resolve(const char *name) {
    struct hostent *hp = gethostbyname(name);
    if (hp == NULL) {
        return NULL;
    }
    return (struct in_addr *) (hp->h_addr_list[0]);
}
