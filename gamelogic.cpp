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
    srand(time(NULL));
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
    int highestBet = 0;
    int playersToAct = playerCount;  // ���� ���忡�� �ൿ�ؾ� �ϴ� �÷��̾� ��

    // �ʱ� ����: ��� �÷��̾��� currentBet�� 0���� ����
    for (int i = 0; i < playerCount; i++) {
        players[i].currentBet = 0;
        if (players[i].isActive && players[i].money > 0) {
            activePlayerCount++;
        }
    }

    // ��� �÷��̾ ����� ���߰ų� ������ ������ �ݺ�
    while (playersToAct > 0) {
        for (int i = 0; i < playerCount; i++) {
            // �÷��̾ �̹� �����߰ų� ���� ���� ��� �ǳʶ�
            if (!players[i].isActive || players[i].money <= 0) {
                continue;
            }

            // �÷��̾��� �ൿ ó��
            printf("%s���� �����Դϴ�. ���� ���� �ݾ�: %d, ���Ϸ��� %d�� �ʿ��մϴ�.\n", players[i].name, highestBet, highestBet - players[i].currentBet);
            handlePlayerAction(&players[i], &highestBet, pot);

            // �÷��̾ �����߰ų� ���ÿ� ����ٸ� playersToAct ����
            if (!players[i].isActive || players[i].currentBet >= highestBet) {
                playersToAct--;
            }

            // ��� �÷��̾� �� Ȱ�� ������ �÷��̾� �� Ȯ��
            activePlayerCount = 0;
            for (int j = 0; j < playerCount; j++) {
                if (players[j].isActive && players[j].money > 0) {
                    activePlayerCount++;
                }
            }

            // �� �� ���� ��� ��� ���� ����
            if (activePlayerCount == 1) {
                return;
            }
        }

        // ���尡 ����� ����: ��� �÷��̾ ����� ����ų� �������� ��
        playersToAct = 0;
        for (int i = 0; i < playerCount; i++) {
            if (players[i].isActive && players[i].currentBet < highestBet && players[i].money > 0) {
                playersToAct++;
            }
        }
    }
}

// �÷��̾��� �׼� ó�� �Լ�: ����, ��, ���� ���� ó��
void handlePlayerAction(Player* player, int* highestBet, int* pot) {
    int action;
    bool validAction = false;

    while (!validAction) {
        printf("%s��, �ൿ�� ������ �ּ��� (1: ��, 2: ������, 3: ����, 4: üũ, 5: ����): ", player->name);
        scanf("%d", &action);

        switch (action) {
        case 1: // ��
            if (*highestBet > player->currentBet) {
                int amountToCall = *highestBet - player->currentBet;
                if (player->money >= amountToCall) {
                    player->money -= amountToCall;
                    player->currentBet = *highestBet;
                    *pot += amountToCall;
                    printf("%s���� %d�� ���߽��ϴ�.\n", player->name, amountToCall);
                    validAction = true;
                }
                else {
                    printf("�ܾ��� �����մϴ�. ���θ� �����մϴ�.\n");
                }
            }
            else {
                printf("�̹� ������ ���������ϴ�.\n");
            }
            break;

        case 2: // ������
        {
            int raiseAmount;
            printf("�󸶸� �������Ͻðڽ��ϱ�? (���� �ְ� ����: %d): ", *highestBet);
            scanf("%d", &raiseAmount);
            if (raiseAmount > 0 && player->money >= raiseAmount + (*highestBet - player->currentBet)) {
                int totalBet = *highestBet + raiseAmount;
                player->money -= (totalBet - player->currentBet);
                player->currentBet = totalBet;
                *highestBet = totalBet;
                *pot += (totalBet - player->currentBet);
                printf("%s���� %d��ŭ �������Ͽ� ���� �ְ� ������ %d�Դϴ�.\n", player->name, raiseAmount, *highestBet);
                validAction = true;
            }
            else {
                printf("�ܾ��� �����ϰų� �߸��� �ݾ��Դϴ�.\n");
            }
        }
        break;

        case 3: // ����
            player->isActive = 0;
            printf("%s���� �����߽��ϴ�.\n", player->name);
            validAction = true;
            break;

        case 4: // üũ
            if (*highestBet == player->currentBet) {
                printf("%s���� üũ�߽��ϴ�.\n", player->name);
                validAction = true;
            }
            else {
                printf("üũ�� �� �����ϴ�. ���� ������ ����� �մϴ�.\n");
            }
            break;

        case 5: // ����
        {
            int allInAmount = player->money;
            player->currentBet += allInAmount;
            player->money = 0;
            *pot += allInAmount;
            if (player->currentBet > *highestBet) {
                *highestBet = player->currentBet;
            }
            printf("%s���� �����Ͽ� %d�� �����߽��ϴ�.\n", player->name, allInAmount);
            validAction = true;
        }
        break;

        default:
            printf("�߸��� �Է��Դϴ�. �ٽ� ������ �ּ���.\n");
            break;
        }
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
Player determineWinner(Player players[], int playerCount, Card communityCards[]) {
    Player* winner = NULL;
    HandEvaluation bestEvaluation = { HIGH_CARD, 0, {0} };

    for (int i = 0; i < playerCount; i++) {
        if (players[i].isActive && players[i].money > 0) {
            HandEvaluation playerEvaluation = evaluateHand(players[i].holeCards, communityCards);

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

    // �ݵ�� ���ڰ� �����ؾ� ��
    printf("\n%s���� ���� ���� �ڵ带 ������ �¸��ϼ̽��ϴ�!\n", winner->name);
    return *winner;
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