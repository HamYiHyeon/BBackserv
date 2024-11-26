#include "shared_memory.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define SHM_KEY 1234

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "사용법: %s <player_index>\n", argv[0]);
        return 1;
    }

    int player_index = atoi(argv[1]);
    pid_t my_pid = getpid();

    // 공유 메모리 접근
    int shmid = shmget(SHM_KEY, sizeof(SharedMemory), 0666);
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

    // 이름있는 파이프(FIFO) 연결
    char player_in_fifo[20], player_out_fifo[20];
    snprintf(player_in_fifo, sizeof(player_in_fifo), "player%d_in.fifo", player_index + 1);
    snprintf(player_out_fifo, sizeof(player_out_fifo), "player%d_out.fifo", player_index + 1);

    int fd_in = open(player_in_fifo, O_WRONLY);
    int fd_out = open(player_out_fifo, O_RDONLY);

    if (fd_in == -1 || fd_out == -1) {
        perror("파이프 열기 실패");
        exit(1);
    }

    // 메인 프로세스와 통신 시작
    while (1) {
        char buffer[256];

        // 메인 프로세스로부터 메시지 읽기
        if (read(fd_in, buffer, sizeof(buffer)) > 0) {
            printf("받은 메시지: %s\n", buffer);

            char message[256];
            scanf("%s", message);
            write(fd_out, message, strlen(message) + 1);
        }
    }

    close(fd_in);
    close(fd_out);

    return 0;
}