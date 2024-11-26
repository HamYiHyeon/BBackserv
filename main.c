#include "Card.h"
#include "gamelogic.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define SHM_KEY 1234

typedef struct {
    pid_t player_pids[PLAYER_COUNT];
    int player_connected[PLAYER_COUNT];  // 0: ���� �ȵ�, 1: �����
    int action[5]
} SharedMemory;

int main() {
    int shmid;
    SharedMemory* shm;

    // ���� �޸� ����
    shmid = shmget(SHM_KEY, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget ����");
        exit(1);
    }

    // ���� �޸� ����
    shm = (SharedMemory*)shmat(shmid, NULL, 0);
    if (shm == (SharedMemory*)-1) {
        perror("shmat ����");
        exit(1);
    }

    // �ʱ�ȭ
    for (int i = 0; i < PLAYER_COUNT; i++) {
        shm->player_pids[i] = 0;
        shm->player_connected[i] = 0;
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
    // �÷��̾�� �� �ʱ�ȭ
    Player players[PLAYER_COUNT];
    Card deck[DECK_SIZE];
    Card communityCards[COMMUNITY_CARD_COUNT];
    int currentBet = 0;
    int pot = 0;
    int deckIndex = 0;
    int lastToRaiseIndex = 0;

    // �÷��̾� �ʱ�ȭ
    for (int i = 0; i < PLAYER_COUNT; i++) {
        printf("�÷��̾� %d �̸��� �Է��ϼ���: ", i + 1);
        scanf("%s", players[i].name);
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
        dealHoleCards(players, PLAYER_COUNT, deck, &deckIndex);

        // 2. ���� ���� (PREFLOP)
        printf("\n=== �����÷� ���� ���� ===\n");
        startBettingRound(players, PLAYER_COUNT, &currentBet, &pot, &lastToRaiseIndex);

        // ���� �� ���� �÷��̾ ������ �ٷ� �¸� ó��
        Player* winner = checkForFoldWinner(players, PLAYER_COUNT);
        if (winner != NULL) {
            printf("\n%s���� �������� �ʰ� �����־� �¸��ϼ̽��ϴ�! �ǵ� %d�� �����մϴ�!\n", winner->name, pot);
            winner->money += pot;
            continue;  // ���� ���� �����
        }

        // 3. Ŀ�´�Ƽ ī�� �й� �� �� ���� ����
        for (Round currentRound = FLOP; currentRound <= RIVER; currentRound = (Round)((int)currentRound + 1)) {
            dealCommunityCards(communityCards, deck, &deckIndex, currentRound);

            switch (currentRound) {
            case FLOP:
                printf("\n=== �÷� ���� ���� ===\n");
                break;
            case TURN:
                printf("\n=== �� ���� ���� ===\n");
                break;
            case RIVER:
                printf("\n=== ���� ���� ���� ===\n");
                break;
            default:
                break;
            }

            startBettingRound(players, PLAYER_COUNT, &currentBet, &pot, &lastToRaiseIndex);

            // �� ���� �÷��̾ ������ �ٷ� �¸� ó��
            winner = checkForFoldWinner(players, PLAYER_COUNT);
            if (winner != NULL) {
                printf("\n%s���� �������� �ʰ� �����־� �¸��ϼ̽��ϴ�! �ǵ� %d�� �����մϴ�!\n", winner->name, pot);
                winner->money += pot;
                break;  // Ŀ�´�Ƽ ī�� ���� ���� ����
            }
        }

        // 4. �¸��� ����
        determineWinners(players, PLAYER_COUNT, communityCards, &pot);

        printf("\n");

        // 5. ���� �ʱ�ȭ
        resetGame(players, PLAYER_COUNT);
        currentBet = 0;
        deckIndex = 0;
        shuffleDeck(deck);  // ���� �ٽ� �����Ͽ� ���ο� ���� �غ�
    }

    // ���� ����� ���
    for (int i = 0; i < PLAYER_COUNT; i++) {
        if (players[i].isActive && players[i].money > 0) {
            printf("\n���� ����! ���� ����ڴ� %s�Դϴ�.\n", players[i].name);
            break;
        }
    }

    return 0;
}
