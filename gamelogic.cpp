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
    int highestBet = 0;
    int playersToAct = playerCount;  // 현재 라운드에서 행동해야 하는 플레이어 수

    // 초기 설정: 모든 플레이어의 currentBet을 0으로 설정
    for (int i = 0; i < playerCount; i++) {
        players[i].currentBet = 0;
        if (players[i].isActive && players[i].money > 0) {
            activePlayerCount++;
        }
    }

    // 모든 플레이어가 레이즈에 맞추거나 폴드할 때까지 반복
    while (playersToAct > 0) {
        for (int i = 0; i < playerCount; i++) {
            // 플레이어가 이미 폴드했거나 돈이 없는 경우 건너뜀
            if (!players[i].isActive || players[i].money <= 0) {
                continue;
            }

            // 플레이어의 행동 처리
            printf("%s님의 차례입니다. 현재 베팅 금액: %d, 콜하려면 %d가 필요합니다.\n", players[i].name, highestBet, highestBet - players[i].currentBet);
            handlePlayerAction(&players[i], &highestBet, pot);

            // 플레이어가 폴드했거나 베팅에 맞췄다면 playersToAct 감소
            if (!players[i].isActive || players[i].currentBet >= highestBet) {
                playersToAct--;
            }

            // 모든 플레이어 중 활성 상태인 플레이어 수 확인
            activePlayerCount = 0;
            for (int j = 0; j < playerCount; j++) {
                if (players[j].isActive && players[j].money > 0) {
                    activePlayerCount++;
                }
            }

            // 한 명만 남은 경우 즉시 라운드 종료
            if (activePlayerCount == 1) {
                return;
            }
        }

        // 라운드가 종료될 조건: 모든 플레이어가 레이즈에 맞췄거나 폴드했을 때
        playersToAct = 0;
        for (int i = 0; i < playerCount; i++) {
            if (players[i].isActive && players[i].currentBet < highestBet && players[i].money > 0) {
                playersToAct++;
            }
        }
    }
}

// 플레이어의 액션 처리 함수: 베팅, 콜, 폴드 등을 처리
void handlePlayerAction(Player* player, int* highestBet, int* pot) {
    int action;
    bool validAction = false;

    while (!validAction) {
        printf("%s님, 행동을 선택해 주세요 (1: 콜, 2: 레이즈, 3: 폴드, 4: 체크, 5: 올인): ", player->name);
        scanf("%d", &action);

        switch (action) {
        case 1: // 콜
            if (*highestBet > player->currentBet) {
                int amountToCall = *highestBet - player->currentBet;
                if (player->money >= amountToCall) {
                    player->money -= amountToCall;
                    player->currentBet = *highestBet;
                    *pot += amountToCall;
                    printf("%s님이 %d를 콜했습니다.\n", player->name, amountToCall);
                    validAction = true;
                }
                else {
                    printf("잔액이 부족합니다. 올인만 가능합니다.\n");
                }
            }
            else {
                printf("이미 베팅이 맞춰졌습니다.\n");
            }
            break;

        case 2: // 레이즈
        {
            int raiseAmount;
            printf("얼마를 레이즈하시겠습니까? (현재 최고 베팅: %d): ", *highestBet);
            scanf("%d", &raiseAmount);
            if (raiseAmount > 0 && player->money >= raiseAmount + (*highestBet - player->currentBet)) {
                int totalBet = *highestBet + raiseAmount;
                player->money -= (totalBet - player->currentBet);
                player->currentBet = totalBet;
                *highestBet = totalBet;
                *pot += (totalBet - player->currentBet);
                printf("%s님이 %d만큼 레이즈하여 현재 최고 베팅은 %d입니다.\n", player->name, raiseAmount, *highestBet);
                validAction = true;
            }
            else {
                printf("잔액이 부족하거나 잘못된 금액입니다.\n");
            }
        }
        break;

        case 3: // 폴드
            player->isActive = 0;
            printf("%s님이 폴드했습니다.\n", player->name);
            validAction = true;
            break;

        case 4: // 체크
            if (*highestBet == player->currentBet) {
                printf("%s님이 체크했습니다.\n", player->name);
                validAction = true;
            }
            else {
                printf("체크할 수 없습니다. 현재 베팅을 맞춰야 합니다.\n");
            }
            break;

        case 5: // 올인
        {
            int allInAmount = player->money;
            player->currentBet += allInAmount;
            player->money = 0;
            *pot += allInAmount;
            if (player->currentBet > *highestBet) {
                *highestBet = player->currentBet;
            }
            printf("%s님이 올인하여 %d를 베팅했습니다.\n", player->name, allInAmount);
            validAction = true;
        }
        break;

        default:
            printf("잘못된 입력입니다. 다시 선택해 주세요.\n");
            break;
        }
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

    // 반드시 승자가 존재해야 함
    printf("\n%s님이 가장 높은 핸드를 가지고 승리하셨습니다!\n", winner->name);
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


// 게임 초기화 함수: 플레이어들의 상태를 초기화하고 새로운 게임 준비
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