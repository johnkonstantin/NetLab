#include "socketUtils.h"

int getSendSocketAndAddr(IP_VERSION ipVersion, const char* ipStr, int port, int* sock, void* addr) {
    if (sock == NULL || addr == NULL || ipStr == NULL) {
        return -1;
    }
    int prot;
    int err;
    switch (ipVersion) {
        case IPV4:
            prot = AF_INET;
            memset(addr, 0, sizeof(struct sockaddr_in));
            ((struct sockaddr_in*) addr)->sin_family = AF_INET;
            ((struct sockaddr_in*) addr)->sin_addr.s_addr = inet_addr(ipStr);
            if (((struct sockaddr_in*) addr)->sin_addr.s_addr == -1) {
                return -1;
            }
            ((struct sockaddr_in*) addr)->sin_port = htons(port);
            break;
        case IPV6:
            prot = AF_INET6;
            memset(addr, 0, sizeof(struct sockaddr_in6));
            ((struct sockaddr_in6*) addr)->sin6_family = AF_INET6;
            ((struct sockaddr_in6*) addr)->sin6_port = htons(port);
            err = inet_pton(AF_INET6, ipStr, &((struct sockaddr_in6*) addr)->sin6_addr);
            if (err != 1) {
                return -1;
            }
            break;
        case ERROR:
            return -1;
    }
    *sock = socket(prot, SOCK_DGRAM, 0);
    if (*sock < 0) {
        return -1;
    }
    return 0;
}

int getReceiveSocketAndAddr(IP_VERSION ipVersion, const char* ipStr, int port, int* sock, void* addr) {
    if (sock == NULL || addr == NULL || ipStr == NULL) {
        return -1;
    }
    int prot;
    int err;
    switch (ipVersion) {
        case IPV4:
            prot = AF_INET;
            break;
        case IPV6:
            prot = AF_INET6;
            break;
        case ERROR:
            return -1;
    }
    if (ipVersion == IPV4) {
        memset(addr, 0, sizeof(struct sockaddr_in));
        ((struct sockaddr_in*) addr)->sin_family = prot;
        ((struct sockaddr_in*) addr)->sin_addr.s_addr = htonl(INADDR_ANY);
        ((struct sockaddr_in*) addr)->sin_port = htons(port);
    }
    else {
        memset(addr, 0, sizeof(struct sockaddr_in6));
        ((struct sockaddr_in6*) addr)->sin6_family = prot;
        ((struct sockaddr_in6*) addr)->sin6_addr = in6addr_any;
        ((struct sockaddr_in6*) addr)->sin6_port = htons(port);
    }
    *sock = socket(prot, SOCK_DGRAM, 0);
    int reuse = 1;
    err = setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
    if (err != 0) {
        return -1;
    }
    int addrLen = (ipVersion == IPV4) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
    err = bind(*sock, (const struct sockaddr*) addr, addrLen);
    if (err != 0) {
        return -1;
    }
    if (ipVersion == IPV4) {
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(ipStr);
        if (mreq.imr_multiaddr.s_addr == -1) {
            return -1;
        }
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        err = setsockopt(*sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
        if (err != 0) {
            return -1;
        }
    }
    else {
        struct ipv6_mreq mreq;
        inet_pton(prot, ipStr, &mreq.ipv6mr_multiaddr);
        err = setsockopt(*sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
        if (err != 0) {
            return -1;
        }
    }

    return 0;
}

IP_VERSION getIPType(const char* ip) {
    int res;

    struct in_addr s;
    res = inet_pton(AF_INET, ip, &s);
    if (res != 1) {
        struct in6_addr ss;
        res = inet_pton(AF_INET6, ip, &ss);
        if (res != 1) {
            return ERROR;
        }

        return IPV6;
    }

    return IPV4;
}

int getLocalIPAddr(IP_VERSION ipVersion, int sock, const char* interface, char* ipAddr) {
    if (ipAddr == NULL || interface == NULL || ipVersion == ERROR) {
        return -1;
    }
    char cmd[256];
    if (ipVersion == IPV4) {
        snprintf(cmd, 256,
                 "ifconfig | grep -A 1 %s | tail -n 1 | grep inet | sed 's/^[ \t]*//' | head -c 20 > temp.txt",
                 interface);
    }
    else {
        snprintf(cmd, 256, "grep %s /proc/net/if_inet6 | head -c 32  > temp.txt", interface);
    }
    system(cmd);
    FILE* file = fopen("temp.txt", "r");
    if (file == NULL) {
        return -1;
    }
    char ifconfigRes[256];
    int num = fread(ifconfigRes, sizeof(char), 256, file);
    fclose(file);
    if (num == 0) {
        return -1;
    }
    system("rm temp.txt");
    if (ipVersion == IPV4) {
        sscanf(ifconfigRes, "inet %s", ipAddr);
    }
    else {
        int acc = 0;
        for (int i = 0, j = 0; i < num; ++i) {
            if (acc == 4) {
                ipAddr[j] = ':';
                ++j;
                acc = 0;
                --i;
                continue;
            }
            ipAddr[j] = ifconfigRes[i];
            ++j;
            ++acc;
        }
    }
    printf("%s\n", ipAddr);
    return 0;
}

int checkIPInMulticast(const char* ipAddr) {
    if (ipAddr == NULL) {
        return -1;
    }
    IP_VERSION ipVersion = getIPType(ipAddr);
    struct in6_addr addr;
    int err;
    switch (ipVersion) {
        case IPV4:
            return IN_MULTICAST(inet_addr(ipAddr));
        case IPV6:
            err = inet_pton(AF_INET6, ipAddr, &addr);
            if (err != 1) {
                return 0;
            }
            return IN6_IS_ADDR_MULTICAST(&addr);
        case ERROR:
            return 0;
    }
}
