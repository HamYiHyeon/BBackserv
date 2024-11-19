#define _CRT_SECURE_NO_WARNINGS
#include "Card.h"
#include "gamelogic.h"
#include <stdio.h>


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
        players[i].isAllIn = 0;
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

        // 만약 한 명의 플레이어만 남으면 바로 승리 처리
        Player* winner = checkForFoldWinner(players, PLAYER_COUNT);
        if (winner != NULL) {
            printf("\n%s님이 폴드하지 않고 남아있어 승리하셨습니다! 판돈 %d를 차지합니다!\n", winner->name, pot);
            winner->money += pot;
            continue;  // 게임 루프 재시작
        }

        // 3. 커뮤니티 카드 분배 및 각 라운드 진행
        for (Round currentRound = FLOP; currentRound <= RIVER; currentRound = static_cast<Round>(static_cast<int>(currentRound) + 1)) {
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

            // 한 명의 플레이어만 남으면 바로 승리 처리
            winner = checkForFoldWinner(players, PLAYER_COUNT);
            if (winner != NULL) {
                printf("\n%s님이 폴드하지 않고 남아있어 승리하셨습니다! 판돈 %d를 차지합니다!\n", winner->name, pot);
                winner->money += pot;
                break;  // 커뮤니티 카드 라운드 루프 종료
            }
        }

        // 4. 승리자 판정
        if (winner == NULL) {
            winner = determineWinner(players, PLAYER_COUNT, communityCards);
            printf("\n%s님이 승리하셨습니다! 판돈 %d를 차지합니다!\n", winner->name, pot);
            winner->money += pot;
        }

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
