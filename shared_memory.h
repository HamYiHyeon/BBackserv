#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <sys/types.h>

#define PLAYER_COUNT 4

extern const char* player_in_pipes[PLAYER_COUNT];
extern const char* player_out_pipes[PLAYER_COUNT];

extern int player_in_fds[PLAYER_COUNT];
extern int player_out_fds[PLAYER_COUNT];

typedef struct {
    pid_t player_pids[PLAYER_COUNT];
    int player_connected[PLAYER_COUNT];  // 0: 연결 안됨, 1: 연결됨, 2: 연결 확인 완료
    int winnerIndex;  // 승리자의 인덱스 (-1이면 아직 승자가 정해지지 않음)
} SharedMemory;

#endif
