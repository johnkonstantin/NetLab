#include "print.h"

void* printerThread(void* arg) {

    while (1) {
        system("clear");
        printf("Alive:\n");
        pthread_mutex_lock(&mutex);
        for (size_t i = 0; i < aliveListLen; ++i) {
            printf("IP: %s, uuid: %s\n", head[i].IPADDR, head[i].UUID);
        }
        aliveListLen = 0;
        pthread_mutex_unlock(&mutex);
        usleep(PRINT_INTERVAL_MS * 1000);
    }

    pthread_exit(NULL);
}
