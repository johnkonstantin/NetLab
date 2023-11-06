#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include "dns.h"
#include "constants.h"
#include "../socketUtils/socketUtils.h"

void calcSpeed(double t1, double t2, double begin_time, size_t total_send, size_t cur_send, char* avg_speed, char* cur_speed) {
    double cur = (double)cur_send / (t2 - t1);
    int p = 0;
    while (cur >= 1024.0) {
        cur /= 1024.0;
        ++p;
    }
    char c;
    switch (p) {
        case 1:
            c = 'k';
            break;
        case 2:
            c = 'M';
            break;
        case 3:
            c = 'G';
            break;
        case 4:
            c = 'T';
            break;
        default:
            c = 'H';
            break;
    }
    if (p == 0) {
        snprintf(cur_speed, 255, "%.3lf B/s", cur);
    }
    else {
        snprintf(cur_speed, 255, "%.3lf %ciB/s", cur, c);
    }
    double avg = (double)total_send / (t2 - begin_time);
    p = 0;
    while (avg >= 1024.0) {
        avg /= 1024.0;
        ++p;
    }
    switch (p) {
        case 1:
            c = 'k';
            break;
        case 2:
            c = 'M';
            break;
        case 3:
            c = 'G';
            break;
        case 4:
            c = 'T';
            break;
        default:
            c = 'H';
            break;
    }
    if (p == 0) {
        snprintf(avg_speed, 255, "%.3lf B/s", avg);
    }
    else {
        snprintf(avg_speed, 255, "%.3lf %ciB/s", avg, c);
    }
}

int main(int argc, char** argv) {
    if (argc != 4) {
        printf("Type server address, port and file path!\n");
        return 0;
    }
    struct in_addr* addr = dns_resolve(argv[1]);
    if (addr == NULL) {
        printf("Wrong address!\n");
        return 0;
    }
    int port = atoi(argv[2]);
    if (port <= 0) {
        printf("Wrong port!\n");
        return 0;
    }
    char* file_path = argv[3];
    char* file_name = file_path;
    for (int i = strlen(file_path) - 1; i >= 0; --i) {
        if (file_path[i] == '/') {
            file_name = file_path + i + 1;
            break;
        }
    }

    int server_sock = getClientSocket(*addr, port);
    if (server_sock < 0) {
        fprintf(stderr, "Could not connect to server!\n");
        return 1;
    }

    ssize_t bytes_num = sendData(server_sock, file_name, strlen(file_name) + 1);
    if (bytes_num == -1) {
        fprintf(stderr, "Could not send data to server!\n");
        return -1;
    }

    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open file!\n");
        return 1;
    }
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    bytes_num = sendData(server_sock, &file_size, sizeof(file_size));
    if (bytes_num == -1) {
        fprintf(stderr, "Could not send data to server!\n");
        return -1;
    }

    char* buf = malloc(NUM_BYTES_TO_SEND);
    size_t send_num = 0;
    size_t t;
    struct timeval tv;
    double t1;
    double t2;
    double last_time;
    double begin_time;
    char avg_str[255];
    char cur_str[255];
    gettimeofday(&tv, NULL);
    begin_time = (double) (long long) (tv.tv_sec) + (double) (long long) (tv.tv_usec) / 10000000.0;
    last_time = (double) (long long) (tv.tv_sec) + (double) (long long) (tv.tv_usec) / 10000000.0 - 100;
    while (send_num < file_size) {
        t = fread(buf, 1, NUM_BYTES_TO_SEND, file);
        gettimeofday(&tv, NULL);
        t1 = (double) (long long) (tv.tv_sec) + (double) (long long) (tv.tv_usec) / 10000000.0;
        size_t acc = 0;
        while (acc != t) {
            bytes_num = sendData(server_sock, buf, t);
            if (bytes_num < 0) {
                fprintf(stderr, "Could not send data to server!\n");
                return -1;
            }
            acc += bytes_num;
        }
        send_num += t;
        gettimeofday(&tv, NULL);
        t2 = (double) (long long) (tv.tv_sec) + (double) (long long) (tv.tv_usec) / 10000000.0;
        if (t2 - last_time >= SPEED_PRINT_INTERVAL_SECONDS) {
            calcSpeed(t1, t2, begin_time, send_num, t, avg_str, cur_str);
            printf("Avg speed: %s, Speed: %s\n", avg_str, cur_str);
            gettimeofday(&tv, NULL);
            last_time = (double) (long long) (tv.tv_sec) + (double) (long long) (tv.tv_usec) / 10000000.0;
        }
    }
    return 0;
}