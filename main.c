#include "Card.h"
#include "gamelogic.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int pipe_fds[PLAYER_COUNT][2];  // 파이프 파일 디스크립터
pid_t player_pids[PLAYER_COUNT];  // 플레이어 프로세스 ID
int current_player_index = -1;

// 자식프로세스가 시그널 받으면 실행할 함수
void signal_handler(int sig) {
    int action;
    printf("행동을 선택하세요: (1) 체크, (2) 콜, (3) 레이즈, (4) 폴드, (5) 올인: ");
    scanf("%d", &action);
    write(pipe_fds[current_player_index][1], &action, sizeof(int)); // 부모 프로세스에 액션 값 전송
}

int main() {
    // 파이프 생성 및 자식 프로세스 생성
    for (int i = 0; i < PLAYER_COUNT; i++) {
        if (pipe(pipe_fds[i]) == -1) {
            perror("pipe failed");
            exit(1);
        }

        if ((player_pids[i] = fork()) == 0) {
            // 자식 프로세스 (플레이어 프로세스)
            close(pipe_fds[i][0]); // 자식은 파이프의 읽기 끝을 닫음
            signal(SIGUSR1, signal_handler);

            current_player_index = i;

            // 메인 루프에서 플레이어의 행동 대기
            while (1) {
                pause();  // 시그널 대기
            }
            perror("child process error");
            exit(1);
        }
        else if (player_pids[i] > 0) {
            // 부모 프로세스
            close(pipe_fds[i][1]); // 부모는 파이프의 쓰기 끝을 닫음
        }
    }
    // 플레이어와 덱 초기화
    Player players[PLAYER_COUNT];
    Card deck[DECK_SIZE];
    Card communityCards[COMMUNITY_CARD_COUNT];
    int currentBet = 0;
    int pot = 0;
    int deckIndex = 0;
    int lastToRaiseIndex = 0;

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
        startBettingRound(players, PLAYER_COUNT, &currentBet, &pot, &lastToRaiseIndex);

        // 만약 한 명의 플레이어만 남으면 바로 승리 처리
        Player* winner = checkForFoldWinner(players, PLAYER_COUNT);
        if (winner != NULL) {
            printf("\n%s님이 폴드하지 않고 남아있어 승리하셨습니다! 판돈 %d를 차지합니다!\n", winner->name, pot);
            winner->money += pot;
            continue;  // 게임 루프 재시작
        }

        // 3. 커뮤니티 카드 분배 및 각 라운드 진행
        for (Round currentRound = FLOP; currentRound <= RIVER; currentRound = (Round)((int)currentRound + 1)) {
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

            startBettingRound(players, PLAYER_COUNT, &currentBet, &pot, &lastToRaiseIndex);

            // 한 명의 플레이어만 남으면 바로 승리 처리
            winner = checkForFoldWinner(players, PLAYER_COUNT);
            if (winner != NULL) {
                printf("\n%s님이 폴드하지 않고 남아있어 승리하셨습니다! 판돈 %d를 차지합니다!\n", winner->name, pot);
                winner->money += pot;
                break;  // 커뮤니티 카드 라운드 루프 종료
            }
        }

        // 4. 승리자 판정
        determineWinners(players, PLAYER_COUNT, communityCards, &pot);

        printf("\n");

        // 5. 게임 초기화
        resetGame(players, PLAYER_COUNT);
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
