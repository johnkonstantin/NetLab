#include "clientHandler.h"

static void calcSpeed(double t1, double t2, double begin_time, size_t total_send, size_t cur_send, char* avg_speed, char* cur_speed) {
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

void* clientHandler(void* args) {
    pthread_detach(pthread_self());

    int sock = *(int*) args;
    free(args);

    char file_name[FILE_NAME_LEN_MAX + 1];
    ssize_t bytes_num = recvData(sock, file_name, FILE_NAME_LEN_MAX + 1);
    if (bytes_num < 0) {
        fprintf(stderr, "Could not receive file path from client!\n");
        close(sock);
        pthread_exit(NULL);
    }
    size_t file_size;
    bytes_num = recvData(sock, &file_size, sizeof(file_size));
    if (bytes_num < 0) {
        fprintf(stderr, "Could not receive file size from client!\n");
        close(sock);
        pthread_exit(NULL);
    }

    printf("New client! File name: %s, file size: %zu\n", file_name, file_size);
    fflush(stdout);

    char file_path[sizeof(SAVE_PATH) + FILE_NAME_LEN_MAX + 1];
    memset(file_path, 0, sizeof(SAVE_PATH) + FILE_NAME_LEN_MAX + 1);
    strcat(file_path, SAVE_PATH);
    strcat(file_path, file_name);
    FILE* file = fopen(file_path, "w+");
    if (file == NULL) {
        fprintf(stderr, "Could not open file!\n");
        close(sock);
        pthread_exit(NULL);
    }
    size_t receive_num = 0;
    size_t fwrite_res;
    char* buf = malloc(NUM_BYTES_TO_RECV);
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
    while (receive_num < file_size) {
        gettimeofday(&tv, NULL);
        t1 = (double) (long long) (tv.tv_sec) + (double) (long long) (tv.tv_usec) / 10000000.0;
        bytes_num = recvData(sock, buf, NUM_BYTES_TO_RECV);
        if (bytes_num < 0) {
            fprintf(stderr, "Could not receive %s data from client! Terminate!\n", file_name);
            close(sock);
            fclose(file);
            free(buf);
            pthread_exit(NULL);
        }
        gettimeofday(&tv, NULL);
        t2 = (double) (long long) (tv.tv_sec) + (double) (long long) (tv.tv_usec) / 10000000.0;
        size_t acc = 0;
        while (acc != bytes_num) {
            fwrite_res = fwrite(buf + acc, 1, bytes_num - acc, file);
            acc += fwrite_res;
        }
        receive_num += acc;
        if (t2 - last_time >= SPEED_PRINT_INTERVAL_SECONDS) {
            calcSpeed(t1, t2, begin_time, receive_num, bytes_num, avg_str, cur_str);
            printf("File %s -> Avg speed: %s, Speed: %s\n", file_name, avg_str, cur_str);
            fflush(stdout);
            gettimeofday(&tv, NULL);
            last_time = (double) (long long) (tv.tv_sec) + (double) (long long) (tv.tv_usec) / 10000000.0;
        }
    }
    printf("File %s received!\n", file_name);
    fflush(stdout);

    close(sock);
    fclose(file);
    free(buf);
    pthread_exit(NULL);
}
