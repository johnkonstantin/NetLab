#include "socketUtils.h"

int getlistetingSocket(int port, int wait_size) {
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(port);

    int listen_sock;
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        return -1;
    }
    if (bind(listen_sock, (const struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        close(listen_sock);
        return -1;
    }
    if (listen(listen_sock, wait_size) < 0) {
        close(listen_sock);
        return -1;
    }
    return listen_sock;
}

int getClientSocket(struct in_addr server_addr, int port) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = server_addr;

    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        return 1;
    }
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }
    return sock;
}

ssize_t sendData(int sock, void* data, size_t size) {
    int ack;
    ssize_t bytes_num = send(sock, data, size, 0);
    if (bytes_num == -1) {
        return -1;
    }
    if (recv(sock, &ack, sizeof(ack), 0) == -1) {
        return -1;
    }
    return bytes_num;
}

ssize_t recvData(int sock, void* data, size_t size) {
    int ack = 1;
    ssize_t bytes_num = recv(sock, data, size, 0);
    if (bytes_num == -1) {
        return -1;
    }
    if (send(sock, &ack, sizeof(ack), 0) == -1) {
        return -1;
    }
    return bytes_num;
}
