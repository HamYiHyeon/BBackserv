#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include "Card.h"

/* ���� ���࿡ ���õ� �����Դϴ�. */

// ��� ����
#define PLAYER_COUNT 4  // �÷��̾� ��
#define COMMUNITY_CARD_COUNT 5 // Ŀ�´�Ƽ ī�� ��

// �÷��̾� ����ü ����
typedef struct {
    char name[50];   // �÷��̾� �̸�
    int money;       // ���� �ݾ�
    Card holeCards[2];  // �� �÷��̾��� Ȧ ī��
    int isActive;    // ���ӿ� ���� ���� (���� ���� ��)
    int currentBet;  // ���� ���忡���� ���� �ݾ�
} Player;

// ���� ���� ����
typedef enum {
    PREFLOP,
    FLOP,
    TURN,
    RIVER
} Round;

// �Լ� ������Ÿ�� ����
void initializeDeck(Card deck[]);  // �� �ʱ�ȭ
void shuffleDeck(Card deck[]);     // �� ����
void dealHoleCards(Player players[], int playerCount, Card deck[], int* deckIndex);  // Ȧ ī�� �й�
void dealCommunityCards(Card communityCards[], Card deck[], int* deckIndex, Round currentRound);  // Ŀ�´�Ƽ ī�� �й�
void startBettingRound(Player players[], int playerCount, int* currentBet, int* pot);  // ���� ���� ����
void handlePlayerAction(Player* player, int* currentBet, int* pot);  // �÷��̾��� �ൿ ó�� (����, ��, ����, üũ, ����)
Player determineWinner(Player players[], int playerCount, Card communityCards[]);  // �¸��� ����
void resetGame(Player players[], int playerCount);  // ���� �ʱ�ȭ
int countActivePlayers(Player players[], int playerCount);  // Ȱ�� �÷��̾� ���� ī��Ʈ
Player* checkForFoldWinner(Player players[], int playerCount);
const char* getRankString(int rank);
const char* getSuitString(int suit);
int compareKickers(int kicker1[], int kicker2[]);


#endif