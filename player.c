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
        fprintf(stderr, "����: %s <player_index>\n", argv[0]);
        return 1;
    }

    int player_index = atoi(argv[1]);
    pid_t my_pid = getpid();

    // ���� �޸� ����
    int shmid = shmget(SHM_KEY, sizeof(SharedMemory), 0666);
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

    // �̸��ִ� ������(FIFO) ����
    char player_in_fifo[20], player_out_fifo[20];
    snprintf(player_in_fifo, sizeof(player_in_fifo), "player%d_in.fifo", player_index + 1);
    snprintf(player_out_fifo, sizeof(player_out_fifo), "player%d_out.fifo", player_index + 1);

    int fd_in = open(player_in_fifo, O_WRONLY);
    int fd_out = open(player_out_fifo, O_RDONLY);

    if (fd_in == -1 || fd_out == -1) {
        perror("������ ���� ����");
        exit(1);
    }

    // ���� ���μ����� ��� ����
    while (1) {
        char buffer[256];

        // ���� ���μ����κ��� �޽��� �б�
        if (read(fd_in, buffer, sizeof(buffer)) > 0) {
            printf("���� �޽���: %s\n", buffer);

            char message[256];
            scanf("%s", message);
            write(fd_out, message, strlen(message) + 1);
        }
    }

    close(fd_in);
    close(fd_out);

    return 0;
}