#ifndef LAB2_CLIENTHANDLER_H
#define LAB2_CLIENTHANDLER_H

#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include "constants.h"
#include "../socketUtils/socketUtils.h"

void* clientHandler(void* args);

#endif //LAB2_CLIENTHANDLER_H
