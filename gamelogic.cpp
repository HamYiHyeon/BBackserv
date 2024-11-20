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

// 홀 카드 분배 함수: 각 플레이어에게 2장의 홀 카드를 분배 (멀티프로세스용)
void dealHoleCardsMulti(Player players[], int playerCount, Card deck[], int* deckIndex, int pipe_fds[][2]) {
    for (int i = 0; i < playerCount; i++) {
        players[i].holeCards[0] = deck[(*deckIndex)++];
        players[i].holeCards[1] = deck[(*deckIndex)++];

        // 플레이어에게 분배된 카드를 파이프를 통해 전달
        write(pipe_fds[i][1], &players[i].holeCards, sizeof(players[i].holeCards));
    }
}

// 커뮤니티 카드 분배 함수: 플랍, 턴, 리버 단계에 따라 커뮤니티 카드를 분배 (멀티프로세스용)
void dealCommunityCardsMulti(Card communityCards[], Card deck[], int* deckIndex, Round currentRound, int pipe_fds[][2], int playerCount) {
    switch (currentRound) {
    case FLOP:
        for (int i = 0; i < 3; i++) {
            communityCards[i] = deck[(*deckIndex)++];
        }
        // 커뮤니티 카드 정보를 모든 플레이어에게 전달
        for (int i = 0; i < playerCount; i++) {
            write(pipe_fds[i][1], &communityCards[0], sizeof(Card) * 3);
        }
        printf("플랍 단계: 커뮤니티 카드 3장이 공개되었습니다: [%s %s], [%s %s], [%s %s]\n",
            getRankString(communityCards[0].rank), getSuitString(communityCards[0].suit),
            getRankString(communityCards[1].rank), getSuitString(communityCards[1].suit),
            getRankString(communityCards[2].rank), getSuitString(communityCards[2].suit));
        break;
    case TURN:
        communityCards[3] = deck[(*deckIndex)++];
        // 커뮤니티 카드 정보를 모든 플레이어에게 전달
        for (int i = 0; i < playerCount; i++) {
            write(pipe_fds[i][1], &communityCards[3], sizeof(Card));
        }
        printf("턴 단계: 커뮤니티 카드 1장이 추가로 공개되었습니다: [%s %s]\n",
            getRankString(communityCards[3].rank), getSuitString(communityCards[3].suit));
        break;
    case RIVER:
        communityCards[4] = deck[(*deckIndex)++];
        // 커뮤니티 카드 정보를 모든 플레이어에게 전달
        for (int i = 0; i < playerCount; i++) {
            write(pipe_fds[i][1], &communityCards[4], sizeof(Card));
        }
        printf("리버 단계: 마지막 커뮤니티 카드 1장이 공개되었습니다: [%s %s]\n",
            getRankString(communityCards[4].rank), getSuitString(communityCards[4].suit));
        break;
    default:
        break;
    }
}

// 멀티프로세스를 위한 베팅 라운드 함수
void startBettingRoundMulti(Player players[], int playerCount, int* currentBet, int* pot, int* lastToRaiseIndex, int pipe_fds[][2]) {
    int activePlayerCount = 0;

    // 초기 설정: 모든 플레이어의 currentBet을 0으로 설정하고, hasCalled를 0으로 초기화
    for (int i = 0; i < playerCount; i++) {
        players[i].currentBet = 0;
        players[i].hasCalled = 0;
        players[i].hasChecked = 0; // 각 플레이어의 체크 여부 초기화
        if (players[i].isActive && players[i].money > 0) {
            activePlayerCount++;
        }
    }

    // 모든 플레이어가 레이즈에 맞추거나 폴드할 때까지 반복
    while (1) {
        int allCalled = 1;  // 모든 플레이어가 레이즈에 맞췄는지 확인하기 위한 변수
        int allChecked = 1; // 모든 플레이어가 체크했는지 확인하기 위한 변수

        // 마지막 레이즈한 플레이어 다음부터 시작
        for (int i = (*lastToRaiseIndex) % playerCount, count = 0; count < playerCount; count++, i = (i + 1) % playerCount) {
            // 플레이어가 이미 폴드했거나 돈이 없는 경우 건너뜀
            if (!players[i].isActive || players[i].money <= 0 || players[i].isAllIn) {
                continue;
            }

            // 플레이어의 행동 처리
            handlePlayerActionMulti(&players[i], currentBet, pot, i, pipe_fds);

            // 행동 후 처리 결과 확인
            if (players[i].currentBet > *currentBet) {
                // 레이즈가 발생한 경우
                *currentBet = players[i].currentBet;
                *lastToRaiseIndex = i;

                // 모든 플레이어의 hasCalled와 hasChecked를 0으로 설정
                for (int j = 0; j < playerCount; j++) {
                    if (players[j].isActive && players[j].money > 0 && !players[j].isAllIn) {
                        players[j].hasCalled = 0;
                        players[j].hasChecked = 0; // 레이즈 시 체크 상태도 초기화
                    }
                }
                players[i].hasCalled = 1; // 레이즈한 플레이어는 이미 콜한 것으로 표시
                allCalled = 0; // 새로운 레이즈가 발생했으므로 다시 allCalled 확인 필요
            }
            else if (players[i].currentBet == *currentBet) {
                // 콜한 경우
                players[i].hasCalled = 1;
            }

            if (players[i].currentBet == 0 && *currentBet == 0) {
                // 체크한 경우
                players[i].hasChecked = 1;
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
        allCalled = 1; // 초기화
        allChecked = 1; // 초기화

        for (int i = 0; i < playerCount; i++) {
            if (players[i].isActive && players[i].hasCalled == 0 && players[i].money > 0 && !players[i].isAllIn) {
                allCalled = 0;
            }
            if (players[i].isActive && players[i].hasChecked == 0 && players[i].money > 0 && !players[i].isAllIn) {
                allChecked = 0;
            }
        }

        // 모든 플레이어가 레이즈에 맞췄다면 반복 종료
        if (allCalled) {
            break;
        }

        // 모든 플레이어가 체크했을 경우 다음 라운드로 넘어감
        if (allChecked && *currentBet == 0) {
            printf("모든 플레이어가 체크했습니다. 다음 라운드로 넘어갑니다.\n");
            break;
        }
    }

    *currentBet = 0; // 다음 라운드를 위해 현재 베팅 금액 초기화
}


// 플레이어의 액션 처리 함수: 베팅, 콜, 폴드 등을 처리
// 멀티프로세스용 플레이어 행동 처리 함수
void handlePlayerActionMulti(Player* player, int* currentBet, int* pot, int playerIndex, int pipe_fds[][2]) {
    int action;

    // 플레이어로부터 행동 입력 받기
    printf("%s님의 행동 선택: (1) 체크, (2) 콜, (3) 레이즈, (4) 폴드, (5) 올인: ", player->name);
    scanf("%d", &action);

    // 선택한 행동 처리
    switch (action) {
    case 1: // 체크
        if (*currentBet == player->currentBet) {
            printf("%s님이 체크하셨습니다.\n", player->name);
            player->hasChecked = 1;
        }
        else {
            printf("체크를 할 수 없습니다. 현재 베팅 금액이 있습니다.\n");
            handlePlayerActionMulti(player, currentBet, pot, playerIndex, pipe_fds); // 재시도
        }
        break;

    case 2: // 콜
        if (*currentBet == 0) {
            printf("현재 베팅 금액이 0이므로 체크만 가능합니다.\n");
            handlePlayerActionMulti(player, currentBet, pot, playerIndex, pipe_fds); // 재시도
        }
        else if (player->money >= *currentBet - player->currentBet) {
            int amountToCall = *currentBet - player->currentBet;
            player->money -= amountToCall;
            *pot += amountToCall;
            player->currentBet = *currentBet;
            player->hasCalled = 1;
            printf("%s님이 콜하셨습니다.\n", player->name);
        }
        else {
            printf("콜을 할 수 없습니다. 돈이 부족합니다. 올인을 선택해야 합니다.\n");
            handlePlayerActionMulti(player, currentBet, pot, playerIndex, pipe_fds); // 재시도
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
            player->currentBet = *currentBet + raiseAmount;
            printf("%s님이 %d만큼 레이즈하셨습니다.\n", player->name, raiseAmount);
        }
        else {
            printf("레이즈를 할 수 없습니다. 돈이 부족합니다.\n");
            handlePlayerActionMulti(player, currentBet, pot, playerIndex, pipe_fds); // 재시도
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
        handlePlayerActionMulti(player, currentBet, pot, playerIndex, pipe_fds); // 재시도
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
void determineWinners(Player players[], int playerCount, Card communityCards[], int* pot) {
    HandEvaluation bestEvaluation = { HIGH_CARD, 0, {0} };
    Player* winners[PLAYER_COUNT];
    int winnerCount = 0;

    for (int i = 0; i < playerCount; i++) {
        if (players[i].isActive) {
            HandEvaluation playerEvaluation = evaluateHand(players[i].holeCards, communityCards);

            // 디버깅 출력
            printf("%s님의 핸드 평가 결과: 랭크 = %d, 최고 카드 = %d\n", players[i].name, playerEvaluation.rank, playerEvaluation.highCard);
            printf("%s님의 키커 카드 : ", players[i].name);
            for (int k = 0; k < 5; k++) {
                printf("%d ", playerEvaluation.kicker[k]);
            }
            printf("\n");

            // 현재의 최고 패와 비교하여 업데이트
            if (winnerCount == 0 ||
                playerEvaluation.rank > bestEvaluation.rank ||
                (playerEvaluation.rank == bestEvaluation.rank && playerEvaluation.highCard > bestEvaluation.highCard) ||
                (playerEvaluation.rank == bestEvaluation.rank && playerEvaluation.highCard == bestEvaluation.highCard &&
                    compareKickers(playerEvaluation.kicker, bestEvaluation.kicker) > 0)) {

                // 새로운 최고 패가 등장하면 우승자 목록을 초기화하고 업데이트
                bestEvaluation = playerEvaluation;
                winners[0] = &players[i];
                winnerCount = 1;
            }
            else if (playerEvaluation.rank == bestEvaluation.rank &&
                playerEvaluation.highCard == bestEvaluation.highCard &&
                compareKickers(playerEvaluation.kicker, bestEvaluation.kicker) == 0) {

                // 동일한 최고 패를 가진 플레이어 추가
                winners[winnerCount++] = &players[i];
            }
        }
    }

    if (winnerCount > 1) {
        int splitPot = *pot / winnerCount;
        for (int i = 0; i < winnerCount; i++) {
            winners[i]->money += splitPot;
            printf("\n%s님이 공동 승리하여 %d의 판돈을 받았습니다!\n", winners[i]->name, splitPot);
        }
    }
    else if (winnerCount == 1) {
        winners[0]->money += *pot;
        printf("\n%s님이 가장 높은 핸드를 가지고 승리하셨습니다! 판돈 %d를 차지합니다!\n", winners[0]->name, *pot);
    }
    else {
        printf("\n승리자가 없습니다. 확인이 필요합니다.\n");
    }

    // Pot 초기화
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