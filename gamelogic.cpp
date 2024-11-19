#define _CRT_SECURE_NO_WARNINGS
#include "gamelogic.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>




// 덱 초기화 함수: 52장의 카드를 초기화
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

// 덱 셔플 함수
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
    case 0: return "스페이드";
    case 1: return "하트";
    case 2: return "다이아몬드";
    case 3: return "클럽";
    default: return "?";
    }
}


// 홀 카드 분배 함수: 각 플레이어에게 2장의 홀 카드를 분배
void dealHoleCards(Player players[], int playerCount, Card deck[], int* deckIndex) {
    for (int i = 0; i < playerCount; i++) {
        players[i].holeCards[0] = deck[(*deckIndex)++];
        players[i].holeCards[1] = deck[(*deckIndex)++];
        // 플레이어에게 분배된 카드 출력
        printf("%s님의 홀 카드: [%s %s], [%s %s]\n",
            players[i].name,
            getRankString(players[i].holeCards[0].rank), getSuitString(players[i].holeCards[0].suit),
            getRankString(players[i].holeCards[1].rank), getSuitString(players[i].holeCards[1].suit));
    }
}


// 커뮤니티 카드 분배 함수: 플랍, 턴, 리버 단계에 따라 커뮤니티 카드를 분배
void dealCommunityCards(Card communityCards[], Card deck[], int* deckIndex, Round currentRound) {
    switch (currentRound) {
    case FLOP:
        for (int i = 0; i < 3; i++) {
            communityCards[i] = deck[(*deckIndex)++];
        }
        printf("플랍 단계: 커뮤니티 카드 3장이 공개되었습니다: [%s %s], [%s %s], [%s %s]\n",
            getRankString(communityCards[0].rank), getSuitString(communityCards[0].suit),
            getRankString(communityCards[1].rank), getSuitString(communityCards[1].suit),
            getRankString(communityCards[2].rank), getSuitString(communityCards[2].suit));
        break;
    case TURN:
        communityCards[3] = deck[(*deckIndex)++];
        printf("턴 단계: 커뮤니티 카드 1장이 추가로 공개되었습니다: [%s %s]\n",
            getRankString(communityCards[3].rank), getSuitString(communityCards[3].suit));
        break;
    case RIVER:
        communityCards[4] = deck[(*deckIndex)++];
        printf("리버 단계: 마지막 커뮤니티 카드 1장이 공개되었습니다: [%s %s]\n",
            getRankString(communityCards[4].rank), getSuitString(communityCards[4].suit));
        break;
    default:
        break;
    }
}


// 베팅 라운드 진행 함수: 각 플레이어가 베팅, 콜, 폴드 등의 액션을 진행
void startBettingRound(Player players[], int playerCount, int* currentBet, int* pot) {
    int activePlayerCount = 0;
    int playersToAct = playerCount;
    int lastToRaiseIndex = -1; // 마지막으로 레이즈한 플레이어의 인덱스

    // 초기 설정: 모든 플레이어의 currentBet을 0으로 설정하고, hasCalled를 0으로 초기화
    for (int i = 0; i < playerCount; i++) {
        players[i].currentBet = 0;
        players[i].hasCalled = 0;
        if (players[i].isActive && players[i].money > 0) {
            activePlayerCount++;
        }
    }

    // 모든 플레이어가 레이즈에 맞추거나 폴드할 때까지 반복
    while (1) {
        int allCalled = 1;  // 모든 플레이어가 레이즈에 맞췄는지 확인하기 위한 변수

        // 마지막 레이즈한 플레이어 다음부터 시작
        for (int i = (lastToRaiseIndex + 1) % playerCount, count = 0; count < playerCount; count++, i = (i + 1) % playerCount) {
            // 플레이어가 이미 폴드했거나 돈이 없는 경우 건너뜀
            if (!players[i].isActive || players[i].money <= 0 || players[i].isAllIn) {
                continue;
            }

            // 마지막 레이즈 이후 모든 플레이어가 콜했으면 반복 종료
            if (players[i].hasCalled == 1) {
                continue;
            }

            // 플레이어의 행동 처리
            int amountToCall = *currentBet - players[i].currentBet;
            printf("%s님의 차례입니다. 현재 베팅 금액: %d, 콜하려면 %d가 필요합니다.\n", players[i].name, *currentBet, amountToCall);
            handlePlayerAction(&players[i], currentBet, pot);

            // 레이즈가 발생한 경우, 마지막으로 레이즈한 플레이어 기록 및 모든 플레이어의 hasCalled 초기화
            if (players[i].currentBet > *currentBet) {
                *currentBet = players[i].currentBet;
                lastToRaiseIndex = i;

                // 모든 플레이어의 hasCalled를 0으로 설정
                for (int j = 0; j < playerCount; j++) {
                    if (players[j].isActive && players[j].money > 0 && !players[j].isAllIn) {
                        players[j].hasCalled = 0;
                    }
                }
                players[i].hasCalled = 1; // 레이즈한 플레이어는 이미 콜한 것으로 표시
            }
            else if (players[i].currentBet == *currentBet) {
                players[i].hasCalled = 1; // 콜했음을 표시
            }

            // 한 명만 남은 경우 즉시 라운드 종료
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

        // 모든 플레이어가 레이즈에 맞췄거나 폴드했는지 확인
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

    *currentBet = 0; // 다음 라운드를 위해 현재 베팅 금액 초기화
}



// 플레이어의 액션 처리 함수: 베팅, 콜, 폴드 등을 처리
void handlePlayerAction(Player* player, int* currentBet, int* pot) {
    int action;
    printf("행동을 선택하세요: (1) 체크, (2) 콜, (3) 레이즈, (4) 폴드, (5) 올인: ");
    scanf("%d", &action);

    switch (action) {
    case 1: // 체크
        if (*currentBet == player->currentBet) {
            printf("%s님이 체크하셨습니다.\n", player->name);
        }
        else {
            printf("체크를 할 수 없습니다. 현재 베팅 금액이 있습니다.\n");
            handlePlayerAction(player, currentBet, pot); // 다시 선택하도록 재귀 호출
        }
        break;

    case 2: // 콜
        if (*currentBet == 0) {
            // 현재 베팅 금액이 0일 때는 콜이 아닌 체크만 가능함
            printf("현재 베팅 금액이 0이므로 체크만 가능합니다.\n");
            handlePlayerAction(player, currentBet, pot);
        }
        else if (player->money >= *currentBet - player->currentBet) {
            int amountToCall = *currentBet - player->currentBet;
            player->money -= amountToCall;
            *pot += amountToCall;
            player->currentBet = *currentBet;
            player->hasCalled = 1;  // 콜을 했음을 표시
            printf("%s님이 콜하셨습니다.\n", player->name);
        }
        else {
            printf("콜을 할 수 없습니다. 돈이 부족합니다. 올인을 선택해야 합니다.\n");
            handlePlayerAction(player, currentBet, pot); // 다시 선택하도록 재귀 호출
        }
        break;

    case 3: // 레이즈
    {
        int raiseAmount;
        printf("얼마를 레이즈 하시겠습니까?: ");
        scanf("%d", &raiseAmount);
        if (player->money >= *currentBet - player->currentBet + raiseAmount) {
            *pot += *currentBet - player->currentBet + raiseAmount;
            player->money -= *currentBet - player->currentBet + raiseAmount;
            player->currentBet += raiseAmount;
            printf("%s님이 %d만큼 레이즈하셨습니다.\n", player->name, raiseAmount);
        }
        else {
            printf("레이즈를 할 수 없습니다. 돈이 부족합니다.\n");
            handlePlayerAction(player, currentBet, pot); // 다시 선택하도록 재귀 호출
        }
    }
    break;

    case 4: // 폴드
        player->isActive = 0;
        printf("%s님이 폴드하셨습니다.\n", player->name);
        break;

    case 5: // 올인
        *pot += player->money;
        player->currentBet += player->money;
        player->money = 0;
        *currentBet = (player->currentBet > *currentBet) ? player->currentBet : *currentBet;
        player->isAllIn = 1;
        printf("%s님이 올인하셨습니다.\n", player->name);
        break;

    default:
        printf("잘못된 입력입니다. 다시 선택해주세요.\n");
        handlePlayerAction(player, currentBet, pot); // 다시 선택하도록 재귀 호출
        break;
    }
}



// 메인서버에서 while (countActivePlayers(players, PLAYER_COUNT) > 1) { 이곳에 게임을 진행하는 코드 작성해주시면 됩니다 }
int countActivePlayers(Player players[], int playerCount) {
    int activeCount = 0;
    for (int i = 0; i < playerCount; i++) {
        if (players[i].isActive && players[i].money > 0) {
            activeCount++;
        }
    }
    return activeCount;
}

// 한명을 제외한 모든 플레이어가 폴드 했을 시 한명을 승리자로 결정(플레이어가 폴드 할 떄마다 호출해주시면 됩니다)
Player* checkForFoldWinner(Player players[], int playerCount) {
    int activePlayerCount = 0;
    Player* lastPlayer = NULL;

    for (int i = 0; i < playerCount; i++) {
        if (players[i].isActive && players[i].money > 0) {
            activePlayerCount++;
            lastPlayer = &players[i];
        }
    }

    // 활성 상태인 플레이어가 한 명이면 그 플레이어가 승자
    if (activePlayerCount == 1) {
        printf("\n%s님이 폴드하지 않고 남아있어 승리하셨습니다!\n", lastPlayer->name);
        return lastPlayer;
    }

    // 아직 승자가 정해지지 않았을 경우 NULL 반환
    return NULL;
}


// 승리자 판정 함수: 커뮤니티 카드와 플레이어의 홀 카드를 사용해 승리자 결정 (배팅라운드 종료 후 2명이상의 플레이어가 남았을 시에 패를 비교하여 승리자 결정)
Player* determineWinner(Player players[], int playerCount, Card communityCards[]) {
    Player* winner = NULL;
    HandEvaluation bestEvaluation = { HIGH_CARD, 0, {0} };

    for (int i = 0; i < playerCount; i++) {
        if (players[i].isActive) {
            HandEvaluation playerEvaluation = evaluateHand(players[i].holeCards, communityCards);

            // 디버깅 출력
            printf("%s님의 핸드 평가 결과: 랭크 = %d, 최고 카드 = %d\n", players[i].name, playerEvaluation.rank, playerEvaluation.highCard);

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
        printf("\n%s님이 가장 높은 핸드를 가지고 승리하셨습니다!\n", winner->name);
    }
    else {
        printf("\n승리자가 없습니다. 확인이 필요합니다.\n");
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


// 게임 초기화 함수: 플레이어들의 상태를 초기화하고 새로운 게임 준비
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
아래 코드는 메인 서버 구현 예시입니다.
방향을 잡아드리기 위해 작성한 예시이고 이런 느낌으로 게임 진행 프로세스 작성해주시면 될 것 같습니다.
(완벽한 코드 아닙니다! checkfolder도 포함되지 않았습니다. 아예 싹 바꾸셔도 괜찮습니다.)
추가로 플레이어 클라이언트 프로세스도 할당하여 진행하는 코드 작성해주시면 감사하겠습니다.
*/
/*
#include "gamelogic.h"
#include "card.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    // 플레이어와 덱 초기화
    Player players[PLAYER_COUNT];
    Card deck[DECK_SIZE];
    Card communityCards[COMMUNITY_CARD_COUNT];
    int currentBet = 0;
    int pot = 0;
    int deckIndex = 0;

    // 플레이어 초기화
    for (int i = 0; i < PLAYER_COUNT; i++) {
        printf("플레이어 %d 이름을 입력하세요: ", i + 1);
        scanf("%s", players[i].name);
        players[i].money = 1000;  // 각 플레이어 초기 금액
        players[i].isActive = 1;  // 모든 플레이어는 처음에 활성 상태
    }

    // 덱 초기화 및 셔플
    initializeDeck(deck);
    shuffleDeck(deck);

    // 게임 진행 루프
    while (countActivePlayers(players, PLAYER_COUNT) > 1) {
        // 1. 홀 카드 분배
        dealHoleCards(players, PLAYER_COUNT, deck, &deckIndex);

        // 2. 베팅 라운드 (PREFLOP)
        printf("\n=== 프리플롭 베팅 라운드 ===\n");
        startBettingRound(players, PLAYER_COUNT, &currentBet, &pot);

        // 3. 커뮤니티 카드 분배 및 각 라운드 진행
        for (Round currentRound = FLOP; currentRound <= RIVER; currentRound++) {
            dealCommunityCards(communityCards, deck, &deckIndex, currentRound);

            switch (currentRound) {
                case FLOP:
                    printf("\n=== 플롭 베팅 라운드 ===\n");
                    break;
                case TURN:
                    printf("\n=== 턴 베팅 라운드 ===\n");
                    break;
                case RIVER:
                    printf("\n=== 리버 베팅 라운드 ===\n");
                    break;
                default:
                    break;
            }

            startBettingRound(players, PLAYER_COUNT, &currentBet, &pot);
        }

        // 4. 승리자 판정
        Player winner = determineWinner(players, PLAYER_COUNT, communityCards);
        printf("\n%s님이 승리하셨습니다! 판돈 %d를 차지합니다!\n", winner.name, pot);
        winner.money += pot;

        // 5. 게임 초기화
        resetGame(players, PLAYER_COUNT);
        pot = 0;
        currentBet = 0;
        deckIndex = 0;
        shuffleDeck(deck);  // 덱을 다시 셔플하여 새로운 게임 준비
    }

    // 최종 우승자 출력
    for (int i = 0; i < PLAYER_COUNT; i++) {
        if (players[i].isActive && players[i].money > 0) {
            printf("\n게임 종료! 최종 우승자는 %s입니다.\n", players[i].name);
            break;
        }
    }

    return 0;
}
*/