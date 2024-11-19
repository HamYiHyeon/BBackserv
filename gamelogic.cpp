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


// Ȧ ī�� �й� �Լ�: �� �÷��̾�� 2���� Ȧ ī�带 �й�
void dealHoleCards(Player players[], int playerCount, Card deck[], int* deckIndex) {
    for (int i = 0; i < playerCount; i++) {
        players[i].holeCards[0] = deck[(*deckIndex)++];
        players[i].holeCards[1] = deck[(*deckIndex)++];
        // �÷��̾�� �й�� ī�� ���
        printf("%s���� Ȧ ī��: [%s %s], [%s %s]\n",
            players[i].name,
            getRankString(players[i].holeCards[0].rank), getSuitString(players[i].holeCards[0].suit),
            getRankString(players[i].holeCards[1].rank), getSuitString(players[i].holeCards[1].suit));
    }
}


// Ŀ�´�Ƽ ī�� �й� �Լ�: �ö�, ��, ���� �ܰ迡 ���� Ŀ�´�Ƽ ī�带 �й�
void dealCommunityCards(Card communityCards[], Card deck[], int* deckIndex, Round currentRound) {
    switch (currentRound) {
    case FLOP:
        for (int i = 0; i < 3; i++) {
            communityCards[i] = deck[(*deckIndex)++];
        }
        printf("�ö� �ܰ�: Ŀ�´�Ƽ ī�� 3���� �����Ǿ����ϴ�: [%s %s], [%s %s], [%s %s]\n",
            getRankString(communityCards[0].rank), getSuitString(communityCards[0].suit),
            getRankString(communityCards[1].rank), getSuitString(communityCards[1].suit),
            getRankString(communityCards[2].rank), getSuitString(communityCards[2].suit));
        break;
    case TURN:
        communityCards[3] = deck[(*deckIndex)++];
        printf("�� �ܰ�: Ŀ�´�Ƽ ī�� 1���� �߰��� �����Ǿ����ϴ�: [%s %s]\n",
            getRankString(communityCards[3].rank), getSuitString(communityCards[3].suit));
        break;
    case RIVER:
        communityCards[4] = deck[(*deckIndex)++];
        printf("���� �ܰ�: ������ Ŀ�´�Ƽ ī�� 1���� �����Ǿ����ϴ�: [%s %s]\n",
            getRankString(communityCards[4].rank), getSuitString(communityCards[4].suit));
        break;
    default:
        break;
    }
}


// ���� ���� ���� �Լ�: �� �÷��̾ ����, ��, ���� ���� �׼��� ����
void startBettingRound(Player players[], int playerCount, int* currentBet, int* pot) {
    int activePlayerCount = 0;
    int playersToAct = playerCount;
    int lastToRaiseIndex = -1; // ���������� �������� �÷��̾��� �ε���

    // �ʱ� ����: ��� �÷��̾��� currentBet�� 0���� �����ϰ�, hasCalled�� 0���� �ʱ�ȭ
    for (int i = 0; i < playerCount; i++) {
        players[i].currentBet = 0;
        players[i].hasCalled = 0;
        if (players[i].isActive && players[i].money > 0) {
            activePlayerCount++;
        }
    }

    // ��� �÷��̾ ����� ���߰ų� ������ ������ �ݺ�
    while (1) {
        int allCalled = 1;  // ��� �÷��̾ ����� ������� Ȯ���ϱ� ���� ����

        // ������ �������� �÷��̾� �������� ����
        for (int i = (lastToRaiseIndex + 1) % playerCount, count = 0; count < playerCount; count++, i = (i + 1) % playerCount) {
            // �÷��̾ �̹� �����߰ų� ���� ���� ��� �ǳʶ�
            if (!players[i].isActive || players[i].money <= 0 || players[i].isAllIn) {
                continue;
            }

            // ������ ������ ���� ��� �÷��̾ �������� �ݺ� ����
            if (players[i].hasCalled == 1) {
                continue;
            }

            // �÷��̾��� �ൿ ó��
            int amountToCall = *currentBet - players[i].currentBet;
            printf("%s���� �����Դϴ�. ���� ���� �ݾ�: %d, ���Ϸ��� %d�� �ʿ��մϴ�.\n", players[i].name, *currentBet, amountToCall);
            handlePlayerAction(&players[i], currentBet, pot);

            // ����� �߻��� ���, ���������� �������� �÷��̾� ��� �� ��� �÷��̾��� hasCalled �ʱ�ȭ
            if (players[i].currentBet > *currentBet) {
                *currentBet = players[i].currentBet;
                lastToRaiseIndex = i;

                // ��� �÷��̾��� hasCalled�� 0���� ����
                for (int j = 0; j < playerCount; j++) {
                    if (players[j].isActive && players[j].money > 0 && !players[j].isAllIn) {
                        players[j].hasCalled = 0;
                    }
                }
                players[i].hasCalled = 1; // �������� �÷��̾�� �̹� ���� ������ ǥ��
            }
            else if (players[i].currentBet == *currentBet) {
                players[i].hasCalled = 1; // �������� ǥ��
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
        for (int i = 0; i < playerCount; i++) {
            if (players[i].isActive && players[i].hasCalled == 0 && players[i].money > 0 && !players[i].isAllIn) {
                allCalled = 0;
                break;
            }
        }

        if (allCalled) {
            break;
        }
    }

    *currentBet = 0; // ���� ���带 ���� ���� ���� �ݾ� �ʱ�ȭ
}



// �÷��̾��� �׼� ó�� �Լ�: ����, ��, ���� ���� ó��
void handlePlayerAction(Player* player, int* currentBet, int* pot) {
    int action;
    printf("�ൿ�� �����ϼ���: (1) üũ, (2) ��, (3) ������, (4) ����, (5) ����: ");
    scanf("%d", &action);

    switch (action) {
    case 1: // üũ
        if (*currentBet == player->currentBet) {
            printf("%s���� üũ�ϼ̽��ϴ�.\n", player->name);
        }
        else {
            printf("üũ�� �� �� �����ϴ�. ���� ���� �ݾ��� �ֽ��ϴ�.\n");
            handlePlayerAction(player, currentBet, pot); // �ٽ� �����ϵ��� ��� ȣ��
        }
        break;

    case 2: // ��
        if (*currentBet == 0) {
            // ���� ���� �ݾ��� 0�� ���� ���� �ƴ� üũ�� ������
            printf("���� ���� �ݾ��� 0�̹Ƿ� üũ�� �����մϴ�.\n");
            handlePlayerAction(player, currentBet, pot);
        }
        else if (player->money >= *currentBet - player->currentBet) {
            int amountToCall = *currentBet - player->currentBet;
            player->money -= amountToCall;
            *pot += amountToCall;
            player->currentBet = *currentBet;
            player->hasCalled = 1;  // ���� ������ ǥ��
            printf("%s���� ���ϼ̽��ϴ�.\n", player->name);
        }
        else {
            printf("���� �� �� �����ϴ�. ���� �����մϴ�. ������ �����ؾ� �մϴ�.\n");
            handlePlayerAction(player, currentBet, pot); // �ٽ� �����ϵ��� ��� ȣ��
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
            player->currentBet += raiseAmount;
            printf("%s���� %d��ŭ �������ϼ̽��ϴ�.\n", player->name, raiseAmount);
        }
        else {
            printf("����� �� �� �����ϴ�. ���� �����մϴ�.\n");
            handlePlayerAction(player, currentBet, pot); // �ٽ� �����ϵ��� ��� ȣ��
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
        handlePlayerAction(player, currentBet, pot); // �ٽ� �����ϵ��� ��� ȣ��
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
Player* determineWinner(Player players[], int playerCount, Card communityCards[]) {
    Player* winner = NULL;
    HandEvaluation bestEvaluation = { HIGH_CARD, 0, {0} };

    for (int i = 0; i < playerCount; i++) {
        if (players[i].isActive) {
            HandEvaluation playerEvaluation = evaluateHand(players[i].holeCards, communityCards);

            // ����� ���
            printf("%s���� �ڵ� �� ���: ��ũ = %d, �ְ� ī�� = %d\n", players[i].name, playerEvaluation.rank, playerEvaluation.highCard);

            if (winner == NULL ||
                playerEvaluation.rank > bestEvaluation.rank ||
                (playerEvaluation.rank == bestEvaluation.rank && playerEvaluation.highCard > bestEvaluation.highCard) ||
                (playerEvaluation.rank == bestEvaluation.rank && playerEvaluation.highCard == bestEvaluation.highCard &&
                    compareKickers(playerEvaluation.kicker, bestEvaluation.kicker) > 0)) {
                winner = &players[i];
                bestEvaluation = playerEvaluation;
            }
        }
    }

    if (winner != NULL) {
        printf("\n%s���� ���� ���� �ڵ带 ������ �¸��ϼ̽��ϴ�!\n", winner->name);
    }
    else {
        printf("\n�¸��ڰ� �����ϴ�. Ȯ���� �ʿ��մϴ�.\n");
    }

    return winner;
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



/*
�Ʒ� �ڵ�� ���� ���� ���� �����Դϴ�.
������ ��Ƶ帮�� ���� �ۼ��� �����̰� �̷� �������� ���� ���� ���μ��� �ۼ����ֽø� �� �� �����ϴ�.
(�Ϻ��� �ڵ� �ƴմϴ�! checkfolder�� ���Ե��� �ʾҽ��ϴ�. �ƿ� �� �ٲټŵ� �������ϴ�.)
�߰��� �÷��̾� Ŭ���̾�Ʈ ���μ����� �Ҵ��Ͽ� �����ϴ� �ڵ� �ۼ����ֽø� �����ϰڽ��ϴ�.
*/
/*
#include "gamelogic.h"
#include "card.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

        // 3. Ŀ�´�Ƽ ī�� �й� �� �� ���� ����
        for (Round currentRound = FLOP; currentRound <= RIVER; currentRound++) {
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
        }

        // 4. �¸��� ����
        Player winner = determineWinner(players, PLAYER_COUNT, communityCards);
        printf("\n%s���� �¸��ϼ̽��ϴ�! �ǵ� %d�� �����մϴ�!\n", winner.name, pot);
        winner.money += pot;

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
*/