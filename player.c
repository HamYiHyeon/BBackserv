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
    int player_connected[PLAYER_COUNT];  // 0: ���� �ȵ�, 1: �����

} SharedMemory;

int main(int argc, char* argv[]) {
    int shmid;
    pid_t my_pid = getpid();
    int player_index;

    if (argc != 2) {
        fprintf(stderr, "����: %s <player_index>\n", argv[0]);
        return 1;
    }

    player_index = atoi(argv[1]);

    // ���� �޸� ����
    shmid = shmget(SHM_KEY, sizeof(SharedMemory), 0666);
    if (shmid < 0) {
        perror("shmget ����");
        exit(1);
    }

    SharedMemory* shm = (SharedMemory*)shmat(shmid, NULL, 0);
    if (shm == (SharedMemory*)-1) {
        perror("shmat ����");
        exit(1);
    }

    // �ڽ��� PID�� ���� ���¸� ���� �޸𸮿� ���
    shm->player_pids[player_index] = my_pid;
    shm->player_connected[player_index] = 1;

    printf("�÷��̾� %d ���� �Ϸ�: PID=%d\n", player_index + 1, my_pid);

    // ���� ���� ����
    // ...

    return 0;
}
