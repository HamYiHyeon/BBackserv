#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include "Card.h"
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/* ���� ���࿡ ���õ� �����Դϴ�. */

// ��� ����
#define PLAYER_COUNT 4  // �÷��̾� ��
#define COMMUNITY_CARD_COUNT 5 // Ŀ�´�Ƽ ī�� ��

extern int pipe_fds[PLAYER_COUNT][2]; // �������� ���� �θ�-�ڽ� ���
extern pid_t player_pids[PLAYER_COUNT]; // �÷��̾��� ���μ��� ID

// �÷��̾� ����ü ����
typedef struct {
    char name[50];   // �÷��̾� �̸�
    int money;       // ���� �ݾ�
    Card holeCards[2];  // �� �÷��̾��� Ȧ ī��
    int isActive;    // ���ӿ� ���� ���� (���� ���� ��)
    int isAllIn;     // ���� ����
    int currentBet;  // ���� ���忡���� ���� �ݾ�
    int hasCalled;  // ������ ���� ���� �ߴ��� ���θ� ����
    int hasChecked; // üũ ���� ����
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
void startBettingRound(Player players[], int playerCount, int* currentBet, int* pot, int* lastToRaiseIndex);  // ���� ���� ����
void handlePlayerAction(Player* player, int* currentBet, int* pot, int playerIndex, int pipe_fds[][2], pid_t player_pids[]);  // �÷��̾��� �ൿ ó�� (����, ��, ����, üũ, ����)
void determineWinners(Player players[], int playerCount, Card communityCards[], int* pot);  // �¸��� ����
void resetGame(Player players[], int playerCount);  // ���� �ʱ�ȭ
int countActivePlayers(Player players[], int playerCount);  // Ȱ�� �÷��̾� ���� ī��Ʈ
Player* checkForFoldWinner(Player players[], int playerCount);
const char* getRankString(int rank);
const char* getSuitString(int suit);
int compareKickers(int kicker1[], int kicker2[]);


#endif