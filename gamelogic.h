#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include "Card.h"
#include "shared_memory.h"

/* 게임 진행에 관련된 로직입니다. */

// 상수 정의
#define PLAYER_COUNT 4  // 플레이어 수
#define COMMUNITY_CARD_COUNT 5 // 커뮤니티 카드 수

// 플레이어 구조체 정의
typedef struct {
    char name[50];   // 플레이어 이름
    int money;       // 소지 금액
    Card holeCards[2];  // 각 플레이어의 홀 카드
    int isActive;    // 게임에 참여 여부 (폴드 여부 등)
    int isAllIn;     // 올인 여부
    int currentBet;  // 현재 라운드에서의 베팅 금액
    int hasCalled;  // 레이즈 이후 콜을 했는지 여부를 저장
    int hasChecked; // 체크 여부 저장
} Player;


// 게임 라운드 정의
typedef enum {
    PREFLOP,
    FLOP,
    TURN,
    RIVER
} Round;

// 함수 프로토타입 선언
void initializeDeck(Card deck[]);  // 덱 초기화
void shuffleDeck(Card deck[]);     // 덱 셔플
void dealHoleCards(Player players[], int playerCount, Card deck[], int* deckIndex);  // 홀 카드 분배
void dealCommunityCards(Card communityCards[], Card deck[], int* deckIndex, Round currentRound);  // 커뮤니티 카드 분배
void startBettingRound(Player players[], int playerCount, int* currentBet, int* pot, int* lastToRaiseIndex);  // 베팅 라운드 진행
void handlePlayerAction(Player* player, int* currentBet, int* pot);  // 플레이어의 행동 처리
void determineWinners(Player players[], int playerCount, Card communityCards[], int* pot);  // 승리자 판정
void resetGame(Player players[], int playerCount);  // 게임 초기화
int countActivePlayers(Player players[], int playerCount);  // 활성 플레이어 수를 카운트
Player* checkForFoldWinner(Player players[], int playerCount);
const char* getRankString(int rank);
const char* getSuitString(int suit);
int compareKickers(int kicker1[], int kicker2[]);

#endif
