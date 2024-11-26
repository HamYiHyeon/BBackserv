#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define SHM_KEY 1234
#define PLAYER_COUNT 4

typedef struct {
    pid_t player_pids[PLAYER_COUNT];
    int player_connected[PLAYER_COUNT];  // 0: 연결 안됨, 1: 연결됨

} SharedMemory;

int main(int argc, char* argv[]) {
    int shmid;
    pid_t my_pid = getpid();
    int player_index;

    if (argc != 2) {
        fprintf(stderr, "사용법: %s <player_index>\n", argv[0]);
        return 1;
    }

    player_index = atoi(argv[1]);

    // 공유 메모리 접근
    shmid = shmget(SHM_KEY, sizeof(SharedMemory), 0666);
    if (shmid < 0) {
        perror("shmget 실패");
        exit(1);
    }

    SharedMemory* shm = (SharedMemory*)shmat(shmid, NULL, 0);
    if (shm == (SharedMemory*)-1) {
        perror("shmat 실패");
        exit(1);
    }

    // 자신의 PID와 연결 상태를 공유 메모리에 기록
    shm->player_pids[player_index] = my_pid;
    shm->player_connected[player_index] = 1;

    printf("플레이어 %d 입장 완료: PID=%d\n", player_index + 1, my_pid);

    // 이후 게임 진행
    // ...

    return 0;
}
