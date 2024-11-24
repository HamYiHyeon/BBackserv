#include "Card.h"
#include "gamelogic.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int pipe_fds[PLAYER_COUNT][2];  // ������ ���� ��ũ����
pid_t player_pids[PLAYER_COUNT];  // �÷��̾� ���μ��� ID
int current_player_index = -1;

// �ڽ����μ����� �ñ׳� ������ ������ �Լ�
void signal_handler(int sig) {
    int action;
    printf("�ൿ�� �����ϼ���: (1) üũ, (2) ��, (3) ������, (4) ����, (5) ����: ");
    scanf("%d", &action);
    write(pipe_fds[current_player_index][1], &action, sizeof(int)); // �θ� ���μ����� �׼� �� ����
}

int main() {
    // ������ ���� �� �ڽ� ���μ��� ����
    for (int i = 0; i < PLAYER_COUNT; i++) {
        if (pipe(pipe_fds[i]) == -1) {
            perror("pipe failed");
            exit(1);
        }

        if ((player_pids[i] = fork()) == 0) {
            // �ڽ� ���μ��� (�÷��̾� ���μ���)
            close(pipe_fds[i][0]); // �ڽ��� �������� �б� ���� ����
            signal(SIGUSR1, signal_handler);

            current_player_index = i;

            // ���� �������� �÷��̾��� �ൿ ���
            while (1) {
                pause();  // �ñ׳� ���
            }
            perror("child process error");
            exit(1);
        }
        else if (player_pids[i] > 0) {
            // �θ� ���μ���
            close(pipe_fds[i][1]); // �θ�� �������� ���� ���� ����
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
