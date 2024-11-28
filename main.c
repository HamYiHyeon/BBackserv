#include "shared_memory.h"
#include "Card.h"
#include "gamelogic.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

#define SHM_KEY 1234
int player_in_fds[PLAYER_COUNT];
int player_out_fds[PLAYER_COUNT];

const char* player_in_pipes[PLAYER_COUNT] = {
    "player1_in.fifo", "player2_in.fifo", "player3_in.fifo", "player4_in.fifo"
};
const char* player_out_pipes[PLAYER_COUNT] = {
    "player1_out.fifo", "player2_out.fifo", "player3_out.fifo", "player4_out.fifo"
};

char message[256];
char buffer[256];

int main() {
    int shmid;
    SharedMemory* shm;

    // ���� �޸� ���� �� ����
    shmid = shmget(SHM_KEY, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget ����");
        exit(1);
    }

    shm = (SharedMemory*)shmat(shmid, NULL, 0);
    if (shm == (SharedMemory*)-1) {
        perror("shmat ����");
        exit(1);
    }

    // ���� �޸� �ʱ�ȭ
    for (int i = 0; i < PLAYER_COUNT; i++) {
        shm->player_pids[i] = 0;
        shm->player_connected[i] = 0;
    }

    // ������ ���� �������� ������ �����ϰ� �����ϸ� �������� ����
    for (int i = 0; i < PLAYER_COUNT; i++) {
        if (access(player_in_pipes[i], F_OK) == -1) {
            if (mkfifo(player_in_pipes[i], 0666) == -1) {
                perror("������ ���� ����");
                exit(1);
            }
        }

        if (access(player_out_pipes[i], F_OK) == -1) {
            if (mkfifo(player_out_pipes[i], 0666) == -1) {
                perror("������ ���� ����");
                exit(1);
            }
        }
    }

    // �÷��̾� ���� ���
    int connected_count = 0;
    printf("�÷��̾���� ������ ��ٸ��� �ֽ��ϴ�...\n");
    while (connected_count < PLAYER_COUNT) {
        for (int i = 0; i < PLAYER_COUNT; i++) {
            if (shm->player_connected[i] == 1 && shm->player_pids[i] != 0) {
                printf("�÷��̾� %d ���� �Ϸ�: PID=%d\n", i + 1, shm->player_pids[i]);
                connected_count++;
                shm->player_connected[i] = 2;  // ���� Ȯ�� �Ϸ�
            }
        }
        sleep(1);  // CPU ����� ���̱� ���� ��� ���
    }

    printf("��� �÷��̾ �����߽��ϴ�. ������ �����մϴ�...\n");

    // �� �÷��̾�� ����ϱ� ���� ������ ����
    for (int i = 0; i < PLAYER_COUNT; i++) {
        player_in_fds[i] = open(player_in_pipes[i], O_RDONLY);
        player_out_fds[i] = open(player_out_pipes[i], O_WRONLY);

        if (player_in_fds[i] == -1 || player_out_fds[i] == -1) {
            perror("������ ���� ����");
            exit(1);
        }
    }
    // �÷��̾�� �� �ʱ�ȭ
    Player players[PLAYER_COUNT];
    Card deck[DECK_SIZE];
    Card communityCards[COMMUNITY_CARD_COUNT];
    int currentBet = 0;
    int pot = 0;
    int deckIndex = 0;
    int lastToRaiseIndex = 0;


    // ���⼭���� �ڽ� ���μ��� ����


    pid_t pid = fork();
    if (pid < 0) {
        perror("fork ����");
        exit(1);
    }
    else if (pid == 0) { // �ڽ� ���μ���
        printf("�ڽ� ���μ������� ������ �����մϴ�...\n");

        // �÷��̾� �̸� �Է¹ޱ�
        printf("�÷��̾� �̸� �Է¹޴���..\n");
        for (int i = 0; i < PLAYER_COUNT; i++) {
            snprintf(message, sizeof(message), "INPUT �÷��̾� %d �̸��� �Է��ϼ���: ", i + 1);
            write(player_out_fds[i], message, strlen(message) + 1);

            if (read(player_in_fds[i], buffer, sizeof(buffer)) > 0)
                strcpy(players[i].name, buffer);

            players[i].money = 1000;  // �� �÷��̾� �ʱ� �ݾ�
            players[i].isActive = 1;  // ��� �÷��̾�� ó���� Ȱ�� ����
            players[i].isAllIn = 0;
        }

        // �� �ʱ�ȭ �� ����
        initializeDeck(deck);
        shuffleDeck(deck);

        // ���� ���� ����
        while (countActivePlayers(players, PLAYER_COUNT) > 1) {
            // 1. Ȧ ī�� �й�
            sleep(2);
            dealHoleCards(players, PLAYER_COUNT, deck, &deckIndex);

            // 2. ���� ���� (PREFLOP)
            printf("\n=== �����÷� ���� ���� ===\n");
            sleep(2);
            for (int i = 0; i < PLAYER_COUNT; i++) {
                snprintf(message, sizeof(message), "ROUND \n=== �����÷� ���� ���� ===\n���ʸ� ��ٷ��ּ���.\n");
                write(player_out_fds[i], message, strlen(message) + 1);
            }
            sleep(2);
            startBettingRound(players, PLAYER_COUNT, &currentBet, &pot, &lastToRaiseIndex);

            // ���� �� ���� �÷��̾ ������ �ٷ� �¸� ó��
            sleep(2);
            Player* winner = checkForFoldWinner(players, PLAYER_COUNT);
            if (winner != NULL) {
                printf("\n%s���� �������� �ʰ� �����־� �¸��ϼ̽��ϴ�! �ǵ� %d�� �����մϴ�!\n", winner->name, pot);
                for (int i = 0; i < PLAYER_COUNT; i++) {
                    snprintf(message, sizeof(message), "\n%s���� �������� �ʰ� �����־� �¸��ϼ̽��ϴ�! �ǵ� %d�� �����մϴ�!\n", winner->name, pot);
                    write(player_out_fds[i], message, strlen(message) + 1);
                }
                winner->money += pot;
                continue;  // ���� ���� �����
            }

            // 3. Ŀ�´�Ƽ ī�� �й� �� �� ���� ����
            for (Round currentRound = FLOP; currentRound <= RIVER; currentRound = (Round)((int)currentRound + 1)) {
                dealCommunityCards(communityCards, deck, &deckIndex, currentRound);
                sleep(2);
                switch (currentRound) {
                case FLOP:
                    printf("\n=== �÷� ���� ���� ===\n");
                    for (int i = 0; i < PLAYER_COUNT; i++) {
                        snprintf(message, sizeof(message), "\n=== �÷� ���� ���� ===\n");
                        write(player_out_fds[i], message, strlen(message) + 1);
                    }
                    sleep(2);
                    break;
                case TURN:
                    printf("\n=== �� ���� ���� ===\n");
                    for (int i = 0; i < PLAYER_COUNT; i++) {
                        snprintf(message, sizeof(message), "\n=== �� ���� ���� ===\n");
                        write(player_out_fds[i], message, strlen(message) + 1);
                    }
                    sleep(2);
                    break;
                case RIVER:
                    printf("\n=== ���� ���� ���� ===\n");
                    for (int i = 0; i < PLAYER_COUNT; i++) {
                        snprintf(message, sizeof(message), "\n=== ���� ���� ���� ===\n");
                        write(player_out_fds[i], message, strlen(message) + 1);
                    }
                    sleep(2);
                    break;
                default:
                    break;
                }
                sleep(2);
                startBettingRound(players, PLAYER_COUNT, &currentBet, &pot, &lastToRaiseIndex);
                sleep(2);
                // �� ���� �÷��̾ ������ �ٷ� �¸� ó��
                winner = checkForFoldWinner(players, PLAYER_COUNT);
                sleep(2);
                if (winner != NULL) {
                    printf("\n%s���� �������� �ʰ� �����־� �¸��ϼ̽��ϴ�! �ǵ� %d�� �����մϴ�!\n", winner->name, pot);
                    for (int i = 0; i < PLAYER_COUNT; i++) {
                        snprintf(message, sizeof(message), "\n%s���� �������� �ʰ� �����־� �¸��ϼ̽��ϴ�! �ǵ� %d�� �����մϴ�!\n", winner->name, pot);
                        write(player_out_fds[i], message, strlen(message) + 1);
                    }
                    winner->money += pot;
                    break;  // Ŀ�´�Ƽ ī�� ���� ���� ����
                }
            }
            sleep(2);
            // 4. �¸��� ����
            determineWinners(players, PLAYER_COUNT, communityCards, &pot);

            printf("\n");

            sleep(2);
            // 5. ���� �ʱ�ȭ
            resetGame(players, PLAYER_COUNT);
            currentBet = 0;
            deckIndex = 0;
            shuffleDeck(deck);  // ���� �ٽ� �����Ͽ� ���ο� ���� �غ�
        }
        sleep(2);
        // ���� ����� ���
        for (int i = 0; i < PLAYER_COUNT; i++) {
            if (players[i].isActive && players[i].money > 0) {
                printf("\n���� ����! ���� ����ڴ� %s�Դϴ�.\n", players[i].name);
                for (int i = 0; i < PLAYER_COUNT; i++) {
                    snprintf(message, sizeof(message), "\n���� ����! ���� ����ڴ� %s�Դϴ�.\n", players[i].name);
                    write(player_out_fds[i], message, strlen(message) + 1);
                }
                break;
            }
        }
        exit(0); // ���⿡�� �ڽ� ���μ��� ������
    }

    // �θ� ���μ������� �ڽ� ���μ��� ���� ��ٸ�
    wait(NULL);

    // ���� �޸� detach
    shmdt(shm);

    return 0;
}