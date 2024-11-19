#define _CRT_SECURE_NO_WARNINGS
#include "Card.h"
#include "gamelogic.h"
#include <stdio.h>


int main() {
    // �÷��̾�� �� �ʱ�ȭ
    Player players[PLAYER_COUNT];
    Card deck[DECK_SIZE];
    Card communityCards[COMMUNITY_CARD_COUNT];
    int currentBet = 0;
    int pot = 0;
    int deckIndex = 0;

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
        startBettingRound(players, PLAYER_COUNT, &currentBet, &pot);

        // ���� �� ���� �÷��̾ ������ �ٷ� �¸� ó��
        Player* winner = checkForFoldWinner(players, PLAYER_COUNT);
        if (winner != NULL) {
            printf("\n%s���� �������� �ʰ� �����־� �¸��ϼ̽��ϴ�! �ǵ� %d�� �����մϴ�!\n", winner->name, pot);
            winner->money += pot;
            continue;  // ���� ���� �����
        }

        // 3. Ŀ�´�Ƽ ī�� �й� �� �� ���� ����
        for (Round currentRound = FLOP; currentRound <= RIVER; currentRound = static_cast<Round>(static_cast<int>(currentRound) + 1)) {
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

            startBettingRound(players, PLAYER_COUNT, &currentBet, &pot);

            // �� ���� �÷��̾ ������ �ٷ� �¸� ó��
            winner = checkForFoldWinner(players, PLAYER_COUNT);
            if (winner != NULL) {
                printf("\n%s���� �������� �ʰ� �����־� �¸��ϼ̽��ϴ�! �ǵ� %d�� �����մϴ�!\n", winner->name, pot);
                winner->money += pot;
                break;  // Ŀ�´�Ƽ ī�� ���� ���� ����
            }
        }

        // 4. �¸��� ����
        if (winner == NULL) {
            winner = determineWinner(players, PLAYER_COUNT, communityCards);
            printf("\n%s���� �¸��ϼ̽��ϴ�! �ǵ� %d�� �����մϴ�!\n", winner->name, pot);
            winner->money += pot;
        }

        // 5. ���� �ʱ�ȭ
        resetGame(players, PLAYER_COUNT);
        pot = 0;
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
