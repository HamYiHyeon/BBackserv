#define _CRT_SECURE_NO_WARNINGS
#include "gamelogic.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "shared_memory.h"
#include <unistd.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <string.h>

char message[4096];
char buffer[4096];

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
    case 2: return "2 ";
    case 3: return "3 ";
    case 4: return "4 ";
    case 5: return "5 ";
    case 6: return "6 ";
    case 7: return "7 ";
    case 8: return "8 ";
    case 9: return "9 ";
    case 10: return "10";
    case 11: return "J ";
    case 12: return "Q ";
    case 13: return "K ";
    case 14: return "A ";
    default: return "? ";
    }
}

const char* getSuitString(int suit) {
    switch (suit) {
    case 0: return "\u2660";
    case 1: return "\u2665";
    case 2: return "\u2666";
    case 3: return "\u2663";
    default: return "?";
    }
}

// 홀 카드 분배 함수: 각 플레이어에게 2장의 홀 카드를 분배
void dealHoleCards(Player players[], int playerCount, Card deck[], int* deckIndex) {
    for (int i = 0; i < playerCount; i++) {
        players[i].holeCards[0] = deck[(*deckIndex)++];
        players[i].holeCards[1] = deck[(*deckIndex)++];
        // 플레이어에게 분배된 카드 출력
        printf("\n%s님의 홀 카드: [%s %s], [%s %s]\n",
            players[i].name,
            getRankString(players[i].holeCards[0].rank), getSuitString(players[i].holeCards[0].suit),
            getRankString(players[i].holeCards[1].rank), getSuitString(players[i].holeCards[1].suit));

        printf("┏━━━━━━┓        ┏━━━━━━┓        \n");
        printf("┃%s    ┃        ┃%s    ┃        \n", getRankString(players[i].holeCards[0].rank), getRankString(players[i].holeCards[1].rank));
        printf("┃      ┃        ┃      ┃        \n");
        printf("┃  %s   ┃        ┃  %s   ┃      \n", getSuitString(players[i].holeCards[0].suit), getSuitString(players[i].holeCards[1].suit));
        printf("┃      ┃        ┃      ┃        \n");
        printf("┃    %s┃        ┃    %s┃        \n", getRankString(players[i].holeCards[0].rank), getRankString(players[i].holeCards[1].rank));
        printf("┗━━━━━━┛        ┗━━━━━━┛        \n");

        snprintf(message, sizeof(message), "%s님의 홀 카드: [%s %s], [%s %s]\n"                 
               "┏━━━━━━┓  ┏━━━━━━┓\n"
               "┃%s    ┃  ┃%s    ┃\n"                 
               "┃      ┃  ┃      ┃\n"                 
               "┃  %s   ┃  ┃  %s   ┃\n"
               "┃      ┃  ┃      ┃\n"                 
               "┃    %s┃  ┃    %s┃                           "
               "┗━━━━━━┛  ┗━━━━━━┛\n",
            players[i].name,
            getRankString(players[i].holeCards[0].rank), getSuitString(players[i].holeCards[0].suit),
            getRankString(players[i].holeCards[1].rank), getSuitString(players[i].holeCards[1].suit),
            getRankString(players[i].holeCards[0].rank), getRankString(players[i].holeCards[1].rank),
            getSuitString(players[i].holeCards[0].suit), getSuitString(players[i].holeCards[1].suit),
            getRankString(players[i].holeCards[0].rank), getRankString(players[i].holeCards[1].rank));
        write(player_out_fds[i], message, strlen(message) + 1);

    }
}


// 커뮤니티 카드 분배 함수: 플랍, 턴, 리버 단계에 따라 커뮤니티 카드를 분배
void dealCommunityCards(Card communityCards[], Card deck[], int* deckIndex, Round currentRound) {
    switch (currentRound) {
    case FLOP:
        for (int i = 0; i < 3; i++) {
            communityCards[i] = deck[(*deckIndex)++];
        }
        printf("\n플랍 단계: 커뮤니티 카드 3장이 공개되었습니다: [%s %s], [%s %s], [%s %s]\n",
            getRankString(communityCards[0].rank), getSuitString(communityCards[0].suit),
            getRankString(communityCards[1].rank), getSuitString(communityCards[1].suit),
            getRankString(communityCards[2].rank), getSuitString(communityCards[2].suit));

        printf("┏━━━━━━┓        ┏━━━━━━┓        ┏━━━━━━┓        \n");
        printf("┃%s    ┃        ┃%s    ┃        ┃%s    ┃        \n", getRankString(communityCards[0].rank), getRankString(communityCards[1].rank), getRankString(communityCards[2].rank));
        printf("┃      ┃        ┃      ┃        ┃      ┃        \n");
        printf("┃  %s   ┃        ┃  %s   ┃        ┃  %s   ┃     \n", getSuitString(communityCards[0].suit), getSuitString(communityCards[1].suit), getSuitString(communityCards[2].suit));
        printf("┃      ┃        ┃      ┃        ┃      ┃        \n");
        printf("┃    %s┃        ┃    %s┃        ┃    %s┃        \n", getRankString(communityCards[0].rank), getRankString(communityCards[1].rank), getRankString(communityCards[2].rank));
        printf("┗━━━━━━┛        ┗━━━━━━┛        ┗━━━━━━┛        \n");



        for (int i = 0; i < PLAYER_COUNT; i++) {
            snprintf(message, sizeof(message), "플랍 단계: 커뮤니티 카드 3장이 공개되었습니다: [%s %s], [%s %s], [%s %s]\n"
                    "┏━━━━━━┓  ┏━━━━━━┓  ┏━━━━━━┓\n"
                    "┃%s    ┃  ┃%s    ┃  ┃%s    ┃\n"
                    "┃      ┃  ┃      ┃  ┃      ┃   "
                    "┃  %s   ┃  ┃  %s   ┃  ┃  %s   ┃\n"
                    "┃      ┃  ┃      ┃  ┃      ┃\n"
                    "┃    %s┃  ┃    %s┃  ┃    %s┃\n"
                    "┗━━━━━━┛  ┗━━━━━━┛  ┗━━━━━━┛\n",
                getRankString(communityCards[0].rank), getSuitString(communityCards[0].suit), getRankString(communityCards[1].rank),
                getSuitString(communityCards[1].suit), getRankString(communityCards[2].rank), getSuitString(communityCards[2].suit),
                getRankString(communityCards[0].rank), getRankString(communityCards[1].rank), getRankString(communityCards[2].rank),
                getSuitString(communityCards[0].suit), getSuitString(communityCards[1].suit), getSuitString(communityCards[2].suit),
                getRankString(communityCards[0].rank), getRankString(communityCards[1].rank), getRankString(communityCards[2].rank));
            write(player_out_fds[i], message, strlen(message) + 1);
        }
        break;
    case TURN:
        communityCards[3] = deck[(*deckIndex)++];
        printf("턴 단계: 커뮤니티 카드 1장이 추가로 공개되었습니다: [%s %s]\n",
            getRankString(communityCards[3].rank), getSuitString(communityCards[3].suit));

        printf("┏━━━━━━┓        ┏━━━━━━┓        ┏━━━━━━┓        ┏━━━━━━┓    \n");
        printf("┃%s    ┃        ┃%s    ┃        ┃%s    ┃        ┃%s    ┃\n",getRankString(communityCards[0].rank),getRankString(communityCards[1].rank),getRankString(communityCards[2].rank),getRankString(communityCards[3].rank));
        printf("┃      ┃        ┃      ┃        ┃      ┃        ┃      ┃    \n");
        printf("┃  %s   ┃        ┃  %s   ┃        ┃  %s   ┃        ┃  %s   ┃\n",getSuitString(communityCards[0].suit),getSuitString(communityCards[1].suit),getSuitString(communityCards[2].suit),getSuitString(communityCards[3].suit));
        printf("┃      ┃        ┃      ┃        ┃      ┃        ┃      ┃    \n");
        printf("┃    %s┃        ┃    %s┃        ┃    %s┃        ┃    %s┃\n",getRankString(communityCards[0].rank),getRankString(communityCards[1].rank),getRankString(communityCards[2].rank),getRankString(communityCards[3].rank));
        printf("┗━━━━━━┛        ┗━━━━━━┛        ┗━━━━━━┛        ┗━━━━━━┛    \n");

        for (int i = 0; i < PLAYER_COUNT; i++) {
            snprintf(message, sizeof(message), "\n턴 단계: 커뮤니티 카드 1장이 추가로 공개되었습니다: [%s %s]\n"
                    "┏━━━━━━┓\n"
                    "┃%-6s┃\n"
                    "┃      ┃\n"
                    "┃  %s   ┃\n"
                    "┃      ┃\n"
                    "┃%6s┃\n"
                    "┗━━━━━━┛\n",
                getRankString(communityCards[3].rank), getSuitString(communityCards[3].suit),
                getRankString(communityCards[3].rank), getSuitString(communityCards[3].suit), getRankString(communityCards[3].rank));
            write(player_out_fds[i], message, strlen(message) + 1);
        }
        break;
    case RIVER:
        communityCards[4] = deck[(*deckIndex)++];
        printf("\n리버 단계: 마지막 커뮤니티 카드 1장이 공개되었습니다: [%s %s]\n",
            getRankString(communityCards[4].rank), getSuitString(communityCards[4].suit));

        printf("┏━━━━━━┓        ┏━━━━━━┓        ┏━━━━━━┓        ┏━━━━━━┓        ┏━━━━━━┓\n");
        printf("┃%s    ┃        ┃%s    ┃        ┃%s    ┃        ┃%s    ┃        ┃%s    ┃\n",getRankString(communityCards[0].rank),getRankString(communityCards[1].rank),getRankString(communityCards[2].rank),getRankString(communityCards[3].rank),getRankString(communityCards[4].rank));
        printf("┃      ┃        ┃      ┃        ┃      ┃        ┃      ┃        ┃      ┃\n");
        printf("┃  %s   ┃        ┃  %s   ┃        ┃  %s   ┃        ┃  %s   ┃        ┃  %s   ┃\n",getSuitString(communityCards[0].suit),getSuitString(communityCards[1].suit),getSuitString(communityCards[2].suit),getSuitString(communityCards[3].suit),getSuitString(communityCards[4].suit));
        printf("┃      ┃        ┃      ┃        ┃      ┃        ┃      ┃        ┃      ┃\n");
        printf("┃    %s┃        ┃    %s┃        ┃    %s┃        ┃    %s┃        ┃    %s┃\n",getRankString(communityCards[0].rank),getRankString(communityCards[1].rank),getRankString(communityCards[2].rank),getRankString(communityCards[3].rank),getRankString(communityCards[4].rank));
        printf("┗━━━━━━┛        ┗━━━━━━┛        ┗━━━━━━┛        ┗━━━━━━┛        ┗━━━━━━┛\n");


        for (int i = 0; i < PLAYER_COUNT; i++) {
            snprintf(message, sizeof(message),
                    "\n리버 단계: 마지막 커뮤니티 카드 1장이 공개되었습니다: [%s %s]\n"
                    "┏━━━━━━┓\n"
                    "┃%-6s┃\n"
                    "┃      ┃\n"
                    "┃  %s   ┃\n"
                    "┃      ┃\n"
                    "┃%6s┃\n"
                    "┗━━━━━━┛\n",
                getRankString(communityCards[4].rank), getSuitString(communityCards[4].suit),
                getRankString(communityCards[4].rank), getSuitString(communityCards[4].suit), getRankString(communityCards[4].rank));
			write(player_out_fds[i], message, strlen(message) + 1);
		}
        break;
    default:
        break;
    }
}

// 베팅 라운드 진행 함수: 각 플레이어가 베팅, 콜, 폴드 등의 액션을 진행
void startBettingRound(Player players[], int playerCount, int* currentBet, int* pot, int* lastToRaiseIndex) {
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

            // 마지막 레이즈 이후 모든 플레이어가 콜했으면 반복 종료
            if (players[i].hasCalled == 1) {
                continue;
            }

            // 플레이어의 행동 처리
            int amountToCall = *currentBet - players[i].currentBet;
            sleep(1);
            printf("%s님의 차례입니다. 현재 베팅 금액: %d원, 콜하려면 %d원이 필요합니다.\n", players[i].name, *currentBet, amountToCall);
            snprintf(message, sizeof(message), "%s님의 차례입니다. 현재 베팅 금액: %d, 콜하려면 %d원이 필요합니다.\n", players[i].name, *currentBet, amountToCall);
            write(player_out_fds[i], message, strlen(message) + 1);
            sleep(1);

            handlePlayerAction(&players[i], currentBet, pot, i);

            // 레이즈가 발생한 경우, 마지막으로 레이즈한 플레이어 기록 및 모든 플레이어의 hasCalled 초기화
            if (players[i].currentBet > *currentBet || (players[i].isAllIn && (players[i].currentBet > *currentBet))) {
                if (players[i].currentBet > *currentBet) *currentBet = players[i].currentBet;

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
                players[i].hasCalled = 1; // 콜했음을 표시
            }

            if (amountToCall == 0 && *currentBet == 0) {
                players[i].hasChecked = 1; // 플레이어가 체크했음을 표시
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
            sleep(1);
            break;
        }

        // 모든 플레이어가 체크했을 경우 다음 라운드로 넘어감
        if (allChecked && *currentBet == 0) {
            sleep(1);
            printf("모든 플레이어가 체크했습니다. 다음 라운드로 넘어갑니다.\n");
            for (int i = 0; i < PLAYER_COUNT; i++) {
                snprintf(message, sizeof(message), "모든 플레이어가 체크했습니다. 다음 라운드로 넘어갑니다.\n");
                write(player_out_fds[i], message, strlen(message) + 1);
            }
            break;
        }
    }

    *currentBet = 0; // 다음 라운드를 위해 현재 베팅 금액 초기화
}

// 플레이어의 액션 처리 함수: 베팅, 콜, 폴드 등을 처리
void handlePlayerAction(Player* player, int* currentBet, int* pot, int playerIndex) {
    snprintf(message, sizeof(message), "BET 행동을 선택하세요: (1) 체크, (2) 콜, (3) 레이즈, (4) 폴드, (5) 올인: ");
    write(player_out_fds[playerIndex], message, strlen(message) + 1);

    int action;
    if (read(player_in_fds[playerIndex], &action, sizeof(action)) > 0) {
        switch (action) {
        case 1: // 체크
            if (*currentBet == player->currentBet) {
                printf("%s님이 체크하셨습니다.\n", player->name);
                for (int i = 0; i < PLAYER_COUNT; i++) {
                    snprintf(message, sizeof(message), "%s님이 체크하셨습니다.\n", player->name);
                    write(player_out_fds[i], message, strlen(message) + 1);
                }
                sleep(1);
                snprintf(message, sizeof(message), "다른 플레이어 배팅중..");
                write(player_out_fds[playerIndex], message, strlen(message) + 1);
                sleep(1);
            }
            else {
                printf("체크를 할 수 없습니다. 현재 베팅 금액이 있습니다.\n");
                snprintf(message, sizeof(message), "체크를 할 수 없습니다. 현재 베팅 금액이 있습니다.\n");
                write(player_out_fds[playerIndex], message, strlen(message) + 1);
                sleep(1);
                handlePlayerAction(player, currentBet, pot, playerIndex); // 다시 선택하도록 재귀 호출
            }
            break;

        case 2: // 콜
            if (*currentBet == 0) {
                // 현재 베팅 금액이 0일 때는 콜이 아닌 체크만 가능함
                printf("현재 베팅 금액이 0이므로 체크만 가능합니다.\n");
                snprintf(message, sizeof(message), "현재 베팅 금액이 0이므로 체크만 가능합니다.\n");
                write(player_out_fds[playerIndex], message, strlen(message) + 1);
                sleep(1);
                handlePlayerAction(player, currentBet, pot, playerIndex);
            }
            else if (player->money >= *currentBet - player->currentBet) {
                int amountToCall = *currentBet - player->currentBet;
                player->money -= amountToCall;
                *pot += amountToCall;
                player->currentBet = *currentBet;
                player->hasCalled = 1;  // 콜을 했음을 표시
                printf("%s님이 콜하셨습니다.\n", player->name);
                for (int i = 0; i < PLAYER_COUNT; i++) {
                    snprintf(message, sizeof(message), "%s님이 콜하셨습니다.\n", player->name);
                    write(player_out_fds[i], message, strlen(message) + 1);
                }
                sleep(1);
                snprintf(message, sizeof(message), "당신이 현재 가지고 있는 금액: %d원\n", player->money);
                write(player_out_fds[playerIndex], message, strlen(message) + 1);
                sleep(1);
                snprintf(message, sizeof(message), "다른 플레이어 배팅중..");
                write(player_out_fds[playerIndex], message, strlen(message) + 1);
                sleep(1);
            }
            else {
                printf("콜을 할 수 없습니다. 돈이 부족합니다. 올인을 선택해야 합니다.\n");
                snprintf(message, sizeof(message), "콜을 할 수 없습니다. 돈이 부족합니다. 올인을 선택해야 합니다.\n");
                write(player_out_fds[playerIndex], message, strlen(message) + 1);
                sleep(1);
                handlePlayerAction(player, currentBet, pot, playerIndex); // 다시 선택하도록 재귀 호출
            }
            break;

        case 3: // 레이즈
        {
            int raiseAmount;
            sleep(1);
            printf("레이즈 하셨습니다. 돈 입력중.. ");
            snprintf(message, sizeof(message), "RAISE 얼마를 레이즈 하시겠습니까?: ");
            write(player_out_fds[playerIndex], message, strlen(message) + 1);
            if (read(player_in_fds[playerIndex], &raiseAmount, sizeof(raiseAmount)) > 0) {
                if (player->money >= *currentBet - player->currentBet + raiseAmount) {
                    *pot += *currentBet - player->currentBet + raiseAmount;
                    player->money -= *currentBet - player->currentBet + raiseAmount;
                    player->currentBet += raiseAmount;
                    printf("%s님이 %d원을 레이즈하셨습니다.\n", player->name, raiseAmount);
                    for (int i = 0; i < PLAYER_COUNT; i++) {
                        snprintf(message, sizeof(message), "%s님이 %d원을 레이즈하셨습니다.\n", player->name, raiseAmount);
                        write(player_out_fds[i], message, strlen(message) + 1);
                    }
                    sleep(1);
                    snprintf(message, sizeof(message), "당신이 현재 가지고 있는 금액: %d원\n", player->money);
                    write(player_out_fds[playerIndex], message, strlen(message) + 1);
                    sleep(1);
                    snprintf(message, sizeof(message), "다른 플레이어 배팅중..");
                    write(player_out_fds[playerIndex], message, strlen(message) + 1);
                    sleep(1);
                }
                else {
                    printf("레이즈를 할 수 없습니다. 돈이 부족합니다.\n");
                    snprintf(message, sizeof(message), "레이즈를 할 수 없습니다. 돈이 부족합니다.\n");
                    write(player_out_fds[playerIndex], message, strlen(message) + 1);
                    sleep(1);
                    handlePlayerAction(player, currentBet, pot, playerIndex); // 다시 선택하도록 재귀 호출
                }
            }
        }
        break;

        case 4: // 폴드
            player->isActive = 0;
            printf("%s님이 폴드하셨습니다.\n", player->name);
            for (int i = 0; i < PLAYER_COUNT; i++) {
                snprintf(message, sizeof(message), "%s님이 폴드하셨습니다.\n", player->name);
                write(player_out_fds[i], message, strlen(message) + 1);
            }
            sleep(1);
            snprintf(message, sizeof(message), "다른 플레이어 배팅중..");
            write(player_out_fds[playerIndex], message, strlen(message) + 1);
            sleep(1);
            break;

        case 5: // 올인
            *pot += player->money;
            player->currentBet += player->money;
            player->money = 0;
            *currentBet = (player->currentBet > *currentBet) ? player->currentBet : *currentBet;
            player->isAllIn = 1;
            printf("%s님이 올인하셨습니다.\n", player->name);
            for (int i = 0; i < PLAYER_COUNT; i++) {
                snprintf(message, sizeof(message), "%s님이 올인하셨습니다.\n", player->name);
                write(player_out_fds[i], message, strlen(message) + 1);
            }
            sleep(1);
            snprintf(message, sizeof(message), "다른 플레이어 배팅중..");
            write(player_out_fds[playerIndex], message, strlen(message) + 1);
            sleep(1);
            break;

        default:
            printf("잘못된 입력입니다. 다시 선택해주세요.\n");
            snprintf(message, sizeof(message), "잘못된 입력입니다. 다시 선택해주세요.\n");
            write(player_out_fds[playerIndex], message, strlen(message) + 1);
            sleep(1);
            handlePlayerAction(player, currentBet, pot, playerIndex);
            break;
        }
    }
}

int countActivePlayers(Player players[], int playerCount) {
    int activeCount = 0;
    for (int i = 0; i < playerCount; i++) {
        if (players[i].money > 0) {
            activeCount++;
            players[i].isActive = 1;
        }
    }
    return activeCount;
}

// 한명을 제외한 모든 플레이어가 폴드 했을 시 한명을 승리자로 결정
Player* checkForFoldWinner(Player players[], int playerCount) {
    int activePlayerCount = 0;
    Player* lastPlayer = NULL;

    for (int i = 0; i < playerCount; i++) {
        if (players[i].isActive > 0) {
            activePlayerCount++;
            lastPlayer = &players[i];
        }
    }

    // 활성 상태인 플레이어가 한 명이면 그 플레이어가 승자
    if (activePlayerCount == 1) {
        sleep(1);
        printf("\n%s님이 폴드하지 않고 남아있어 승리하셨습니다!\n", lastPlayer->name);
        for (int i = 0; i < PLAYER_COUNT; i++) {
            snprintf(message, sizeof(message), "\n%s님이 폴드하지 않고 남아있어 승리하셨습니다!\n", lastPlayer->name);
            write(player_out_fds[i], message, strlen(message) + 1);
        }
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
            snprintf(message, sizeof(message), "%s님의 핸드 평가 결과: 랭크 = %d, 최고 카드 = %d\n", players[i].name, playerEvaluation.rank, playerEvaluation.highCard);
            write(player_out_fds[i], message, strlen(message) + 1);
            sleep(1);
            snprintf(message, sizeof(message), "%s님의 키커 카드 : ", players[i].name);
            write(player_out_fds[i], message, strlen(message) + 1);
            sleep(1);
            for (int k = 0; k < 5; k++) {
                printf("%d ", playerEvaluation.kicker[k]);
                snprintf(message, sizeof(message), "%d ", playerEvaluation.kicker[k]);
                write(player_out_fds[i], message, strlen(message) + 1);

                sleep(1);
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
            // 모두에게 출력되도록 구현 필요
            snprintf(message, sizeof(message), "\n%s님이 공동 승리하여 %d의 판돈을 받았습니다!\n", winners[i]->name, splitPot);
            write(player_out_fds[i], message, strlen(message) + 1);
        }
    }
    else if (winnerCount == 1) {
        winners[0]->money += *pot;
        printf("\n%s님이 가장 높은 핸드를 가지고 승리하셨습니다! 판돈 %d를 차지합니다!\n", winners[0]->name, *pot);
        for (int i = 0; i < PLAYER_COUNT; i++) {
            snprintf(message, sizeof(message), "\n%s님이 가장 높은 핸드를 가지고 승리하셨습니다! 판돈 %d를 차지합니다!\n", winners[0]->name, *pot);
            write(player_out_fds[i], message, strlen(message) + 1);
        }
        sleep(2);
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
