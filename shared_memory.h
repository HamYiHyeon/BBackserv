#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <sys/types.h>

#define PLAYER_COUNT 4

typedef struct {
    pid_t player_pids[PLAYER_COUNT];
    int player_connected[PLAYER_COUNT];  // 0: ¿¬°á ¾ÈµÊ, 1: ¿¬°áµÊ
} SharedMemory;

#endif