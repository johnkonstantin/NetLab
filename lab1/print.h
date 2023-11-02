#ifndef LAB1_PRINT_H
#define LAB1_PRINT_H

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "constants.h"
#include "aliveList.h"

extern pthread_mutex_t mutex;
extern Node* head;
extern size_t aliveListLen;

void* printerThread(void* arg);

#endif //LAB1_PRINT_H
