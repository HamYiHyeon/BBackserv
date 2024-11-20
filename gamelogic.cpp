#define _CRT_SECURE_NO_WARNINGS
#include "gamelogic.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

// �� �ʱ�ȭ �Լ�: 52���� ī�带 �ʱ�ȭ
void initializeDeck(Card deck[]) {
    int index = 0;
    for (int suit = 0; suit < 4; suit++) {
        for (int rank = 2; rank <= 14; rank++) {
            deck[index].suit = suit;
            deck[index].rank = rank;
            index++;
        }
    }
}

// �� ���� �Լ�
void shuffleDeck(Card deck[]) {
    srand((unsigned int)time(NULL));
    for (int i = 0; i < DECK_SIZE; i++) {
        int j = rand() % DECK_SIZE;
        Card temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

const char* getRankString(int rank) {
    switch (rank) {
    case 2: return "2";
    case 3: return "3";
    case 4: return "4";
    case 5: return "5";
    case 6: return "6";
    case 7: return "7";
    case 8: return "8";
    case 9: return "9";
    case 10: return "10";
    case 11: return "J";
    case 12: return "Q";
    case 13: return "K";
    case 14: return "A";
    default: return "?";
    }
}

const char* getSuitString(int suit) {
    switch (suit) {
    case 0: return "�����̵�";
    case 1: return "��Ʈ";
    case 2: return "���̾Ƹ��";
    case 3: return "Ŭ��";
    default: return "?";
    }
}

// Ȧ ī�� �й� �Լ�: �� �÷��̾�� 2���� Ȧ ī�带 �й� (��Ƽ���μ�����)
void dealHoleCardsMulti(Player players[], int playerCount, Card deck[], int* deckIndex, int pipe_fds[][2]) {
    for (int i = 0; i < playerCount; i++) {
        players[i].holeCards[0] = deck[(*deckIndex)++];
        players[i].holeCards[1] = deck[(*deckIndex)++];

        // �÷��̾�� �й�� ī�带 �������� ���� ����
        write(pipe_fds[i][1], &players[i].holeCards, sizeof(players[i].holeCards));
    }
}

// Ŀ�´�Ƽ ī�� �й� �Լ�: �ö�, ��, ���� �ܰ迡 ���� Ŀ�´�Ƽ ī�带 �й� (��Ƽ���μ�����)
void dealCommunityCardsMulti(Card communityCards[], Card deck[], int* deckIndex, Round currentRound, int pipe_fds[][2], int playerCount) {
    switch (currentRound) {
    case FLOP:
        for (int i = 0; i < 3; i++) {
            communityCards[i] = deck[(*deckIndex)++];
        }
        // Ŀ�´�Ƽ ī�� ������ ��� �÷��̾�� ����
        for (int i = 0; i < playerCount; i++) {
            write(pipe_fds[i][1], &communityCards[0], sizeof(Card) * 3);
        }
        printf("�ö� �ܰ�: Ŀ�´�Ƽ ī�� 3���� �����Ǿ����ϴ�: [%s %s], [%s %s], [%s %s]\n",
            getRankString(communityCards[0].rank), getSuitString(communityCards[0].suit),
            getRankString(communityCards[1].rank), getSuitString(communityCards[1].suit),
            getRankString(communityCards[2].rank), getSuitString(communityCards[2].suit));
        break;
    case TURN:
        communityCards[3] = deck[(*deckIndex)++];
        // Ŀ�´�Ƽ ī�� ������ ��� �÷��̾�� ����
        for (int i = 0; i < playerCount; i++) {
            write(pipe_fds[i][1], &communityCards[3], sizeof(Card));
        }
        printf("�� �ܰ�: Ŀ�´�Ƽ ī�� 1���� �߰��� �����Ǿ����ϴ�: [%s %s]\n",
            getRankString(communityCards[3].rank), getSuitString(communityCards[3].suit));
        break;
    case RIVER:
        communityCards[4] = deck[(*deckIndex)++];
        // Ŀ�´�Ƽ ī�� ������ ��� �÷��̾�� ����
        for (int i = 0; i < playerCount; i++) {
            write(pipe_fds[i][1], &communityCards[4], sizeof(Card));
        }
        printf("���� �ܰ�: ������ Ŀ�´�Ƽ ī�� 1���� �����Ǿ����ϴ�: [%s %s]\n",
            getRankString(communityCards[4].rank), getSuitString(communityCards[4].suit));
        break;
    default:
        break;
    }
}

// ��Ƽ���μ����� ���� ���� ���� �Լ�
void startBettingRoundMulti(Player players[], int playerCount, int* currentBet, int* pot, int* lastToRaiseIndex, int pipe_fds[][2]) {
    int activePlayerCount = 0;

    // �ʱ� ����: ��� �÷��̾��� currentBet�� 0���� �����ϰ�, hasCalled�� 0���� �ʱ�ȭ
    for (int i = 0; i < playerCount; i++) {
        players[i].currentBet = 0;
        players[i].hasCalled = 0;
        players[i].hasChecked = 0; // �� �÷��̾��� üũ ���� �ʱ�ȭ
        if (players[i].isActive && players[i].money > 0) {
            activePlayerCount++;
        }
    }

    // ��� �÷��̾ ����� ���߰ų� ������ ������ �ݺ�
    while (1) {
        int allCalled = 1;  // ��� �÷��̾ ����� ������� Ȯ���ϱ� ���� ����
        int allChecked = 1; // ��� �÷��̾ üũ�ߴ��� Ȯ���ϱ� ���� ����

        // ������ �������� �÷��̾� �������� ����
        for (int i = (*lastToRaiseIndex) % playerCount, count = 0; count < playerCount; count++, i = (i + 1) % playerCount) {
            // �÷��̾ �̹� �����߰ų� ���� ���� ��� �ǳʶ�
            if (!players[i].isActive || players[i].money <= 0 || players[i].isAllIn) {
                continue;
            }

            // �÷��̾��� �ൿ ó��
            handlePlayerActionMulti(&players[i], currentBet, pot, i, pipe_fds);

            // �ൿ �� ó�� ��� Ȯ��
            if (players[i].currentBet > *currentBet) {
                // ����� �߻��� ���
                *currentBet = players[i].currentBet;
                *lastToRaiseIndex = i;

                // ��� �÷��̾��� hasCalled�� hasChecked�� 0���� ����
                for (int j = 0; j < playerCount; j++) {
                    if (players[j].isActive && players[j].money > 0 && !players[j].isAllIn) {
                        players[j].hasCalled = 0;
                        players[j].hasChecked = 0; // ������ �� üũ ���µ� �ʱ�ȭ
                    }
                }
                players[i].hasCalled = 1; // �������� �÷��̾�� �̹� ���� ������ ǥ��
                allCalled = 0; // ���ο� ����� �߻������Ƿ� �ٽ� allCalled Ȯ�� �ʿ�
            }
            else if (players[i].currentBet == *currentBet) {
                // ���� ���
                players[i].hasCalled = 1;
            }

            if (players[i].currentBet == 0 && *currentBet == 0) {
                // üũ�� ���
                players[i].hasChecked = 1;
            }

            // �� �� ���� ��� ��� ���� ����
            activePlayerCount = 0;
            for (int j = 0; j < playerCount; j++) {
                if (players[j].isActive && (players[j].money > 0 || players[j].isAllIn)) {
                    activePlayerCount++;
                }
            }

            if (activePlayerCount == 1) {
                return;
            }
        }

        // ��� �÷��̾ ����� ����ų� �����ߴ��� Ȯ��
        allCalled = 1; // �ʱ�ȭ
        allChecked = 1; // �ʱ�ȭ

        for (int i = 0; i < playerCount; i++) {
            if (players[i].isActive && players[i].hasCalled == 0 && players[i].money > 0 && !players[i].isAllIn) {
                allCalled = 0;
            }
            if (players[i].isActive && players[i].hasChecked == 0 && players[i].money > 0 && !players[i].isAllIn) {
                allChecked = 0;
            }
        }

        // ��� �÷��̾ ����� ����ٸ� �ݺ� ����
        if (allCalled) {
            break;
        }

        // ��� �÷��̾ üũ���� ��� ���� ����� �Ѿ
        if (allChecked && *currentBet == 0) {
            printf("��� �÷��̾ üũ�߽��ϴ�. ���� ����� �Ѿ�ϴ�.\n");
            break;
        }
    }

    *currentBet = 0; // ���� ���带 ���� ���� ���� �ݾ� �ʱ�ȭ
}


// �÷��̾��� �׼� ó�� �Լ�: ����, ��, ���� ���� ó��
// ��Ƽ���μ����� �÷��̾� �ൿ ó�� �Լ�
void handlePlayerActionMulti(Player* player, int* currentBet, int* pot, int playerIndex, int pipe_fds[][2]) {
    int action;

    // �÷��̾�κ��� �ൿ �Է� �ޱ�
    printf("%s���� �ൿ ����: (1) üũ, (2) ��, (3) ������, (4) ����, (5) ����: ", player->name);
    scanf("%d", &action);

    // ������ �ൿ ó��
    switch (action) {
    case 1: // üũ
        if (*currentBet == player->currentBet) {
            printf("%s���� üũ�ϼ̽��ϴ�.\n", player->name);
            player->hasChecked = 1;
        }
        else {
            printf("üũ�� �� �� �����ϴ�. ���� ���� �ݾ��� �ֽ��ϴ�.\n");
            handlePlayerActionMulti(player, currentBet, pot, playerIndex, pipe_fds); // ��õ�
        }
        break;

    case 2: // ��
        if (*currentBet == 0) {
            printf("���� ���� �ݾ��� 0�̹Ƿ� üũ�� �����մϴ�.\n");
            handlePlayerActionMulti(player, currentBet, pot, playerIndex, pipe_fds); // ��õ�
        }
        else if (player->money >= *currentBet - player->currentBet) {
            int amountToCall = *currentBet - player->currentBet;
            player->money -= amountToCall;
            *pot += amountToCall;
            player->currentBet = *currentBet;
            player->hasCalled = 1;
            printf("%s���� ���ϼ̽��ϴ�.\n", player->name);
        }
        else {
            printf("���� �� �� �����ϴ�. ���� �����մϴ�. ������ �����ؾ� �մϴ�.\n");
            handlePlayerActionMulti(player, currentBet, pot, playerIndex, pipe_fds); // ��õ�
        }
        break;

    case 3: // ������
    {
        int raiseAmount;
        printf("�󸶸� ������ �Ͻðڽ��ϱ�?: ");
        scanf("%d", &raiseAmount);
        if (player->money >= *currentBet - player->currentBet + raiseAmount) {
            *pot += *currentBet - player->currentBet + raiseAmount;
            player->money -= *currentBet - player->currentBet + raiseAmount;
            player->currentBet = *currentBet + raiseAmount;
            printf("%s���� %d��ŭ �������ϼ̽��ϴ�.\n", player->name, raiseAmount);
        }
        else {
            printf("����� �� �� �����ϴ�. ���� �����մϴ�.\n");
            handlePlayerActionMulti(player, currentBet, pot, playerIndex, pipe_fds); // ��õ�
        }
    }
    break;

    case 4: // ����
        player->isActive = 0;
        printf("%s���� �����ϼ̽��ϴ�.\n", player->name);
        break;

    case 5: // ����
        *pot += player->money;
        player->currentBet += player->money;
        player->money = 0;
        *currentBet = (player->currentBet > *currentBet) ? player->currentBet : *currentBet;
        player->isAllIn = 1;
        printf("%s���� �����ϼ̽��ϴ�.\n", player->name);
        break;

    default:
        printf("�߸��� �Է��Դϴ�. �ٽ� �������ּ���.\n");
        handlePlayerActionMulti(player, currentBet, pot, playerIndex, pipe_fds); // ��õ�
        break;
    }
}


// ���μ������� while (countActivePlayers(players, PLAYER_COUNT) > 1) { �̰��� ������ �����ϴ� �ڵ� �ۼ����ֽø� �˴ϴ� }
int countActivePlayers(Player players[], int playerCount) {
    int activeCount = 0;
    for (int i = 0; i < playerCount; i++) {
        if (players[i].isActive && players[i].money > 0) {
            activeCount++;
        }
    }
    return activeCount;
}

// �Ѹ��� ������ ��� �÷��̾ ���� ���� �� �Ѹ��� �¸��ڷ� ����(�÷��̾ ���� �� ������ ȣ�����ֽø� �˴ϴ�)
Player* checkForFoldWinner(Player players[], int playerCount) {
    int activePlayerCount = 0;
    Player* lastPlayer = NULL;

    for (int i = 0; i < playerCount; i++) {
        if (players[i].isActive && players[i].money > 0) {
            activePlayerCount++;
            lastPlayer = &players[i];
        }
    }

    // Ȱ�� ������ �÷��̾ �� ���̸� �� �÷��̾ ����
    if (activePlayerCount == 1) {
        printf("\n%s���� �������� �ʰ� �����־� �¸��ϼ̽��ϴ�!\n", lastPlayer->name);
        return lastPlayer;
    }

    // ���� ���ڰ� �������� �ʾ��� ��� NULL ��ȯ
    return NULL;
}

// �¸��� ���� �Լ�: Ŀ�´�Ƽ ī��� �÷��̾��� Ȧ ī�带 ����� �¸��� ���� (���ö��� ���� �� 2���̻��� �÷��̾ ������ �ÿ� �и� ���Ͽ� �¸��� ����)
void determineWinners(Player players[], int playerCount, Card communityCards[], int* pot) {
    HandEvaluation bestEvaluation = { HIGH_CARD, 0, {0} };
    Player* winners[PLAYER_COUNT];
    int winnerCount = 0;

    for (int i = 0; i < playerCount; i++) {
        if (players[i].isActive) {
            HandEvaluation playerEvaluation = evaluateHand(players[i].holeCards, communityCards);

            // ����� ���
            printf("%s���� �ڵ� �� ���: ��ũ = %d, �ְ� ī�� = %d\n", players[i].name, playerEvaluation.rank, playerEvaluation.highCard);
            printf("%s���� ŰĿ ī�� : ", players[i].name);
            for (int k = 0; k < 5; k++) {
                printf("%d ", playerEvaluation.kicker[k]);
            }
            printf("\n");

            // ������ �ְ� �п� ���Ͽ� ������Ʈ
            if (winnerCount == 0 ||
                playerEvaluation.rank > bestEvaluation.rank ||
                (playerEvaluation.rank == bestEvaluation.rank && playerEvaluation.highCard > bestEvaluation.highCard) ||
                (playerEvaluation.rank == bestEvaluation.rank && playerEvaluation.highCard == bestEvaluation.highCard &&
                    compareKickers(playerEvaluation.kicker, bestEvaluation.kicker) > 0)) {

                // ���ο� �ְ� �а� �����ϸ� ����� ����� �ʱ�ȭ�ϰ� ������Ʈ
                bestEvaluation = playerEvaluation;
                winners[0] = &players[i];
                winnerCount = 1;
            }
            else if (playerEvaluation.rank == bestEvaluation.rank &&
                playerEvaluation.highCard == bestEvaluation.highCard &&
                compareKickers(playerEvaluation.kicker, bestEvaluation.kicker) == 0) {

                // ������ �ְ� �и� ���� �÷��̾� �߰�
                winners[winnerCount++] = &players[i];
            }
        }
    }

    if (winnerCount > 1) {
        int splitPot = *pot / winnerCount;
        for (int i = 0; i < winnerCount; i++) {
            winners[i]->money += splitPot;
            printf("\n%s���� ���� �¸��Ͽ� %d�� �ǵ��� �޾ҽ��ϴ�!\n", winners[i]->name, splitPot);
        }
    }
    else if (winnerCount == 1) {
        winners[0]->money += *pot;
        printf("\n%s���� ���� ���� �ڵ带 ������ �¸��ϼ̽��ϴ�! �ǵ� %d�� �����մϴ�!\n", winners[0]->name, *pot);
    }
    else {
        printf("\n�¸��ڰ� �����ϴ�. Ȯ���� �ʿ��մϴ�.\n");
    }

    // Pot �ʱ�ȭ
    *pot = 0;
}

int compareKickers(int kicker1[], int kicker2[]) {
    for (int i = 0; i < 5; i++) {
        if (kicker1[i] > kicker2[i]) {
            return 1;
        }
        else if (kicker1[i] < kicker2[i]) {
            return -1;
        }
    }
    return 0;
}

// ���� �ʱ�ȭ �Լ�: �÷��̾���� ���¸� �ʱ�ȭ�ϰ� ���ο� ���� �غ�
void resetGame(Player players[], int playerCount) {
    for (int i = 0; i < playerCount; i++) {
        if (players[i].money > 0) {
            players[i].isActive = 1;
            players[i].isAllIn = 0;
        }
        else {
            players[i].isActive = 0;
        }
        players[i].currentBet = 0;
    }
}