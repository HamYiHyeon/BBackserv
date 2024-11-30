#include "shared_memory.h"
#include "Card.h"
#include "gamelogic.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define SHM_KEY 1234
int player_in_fds[PLAYER_COUNT];
int player_out_fds[PLAYER_COUNT];

const char* player_in_pipes[PLAYER_COUNT] = {
    "player1_in.fifo", "player2_in.fifo", "player3_in.fifo", "player4_in.fifo"
};
const char* player_out_pipes[PLAYER_COUNT] = {
    "player1_out.fifo", "player2_out.fifo", "player3_out.fifo", "player4_out.fifo"
};

char message[256];
char buffer[256];

int main() {
    int shmid;
    SharedMemory* shm;

    // 공유 메모리 생성 및 연결
    shmid = shmget(SHM_KEY, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget 실패");
        exit(1);
    }

    shm = (SharedMemory*)shmat(shmid, NULL, 0);
    if (shm == (SharedMemory*)-1) {
        perror("shmat 실패");
        exit(1);
    }

    // 공유 메모리 초기화
    for (int i = 0; i < PLAYER_COUNT; i++) {
        shm->player_pids[i] = 0;
        shm->player_connected[i] = 0;
    }

    // 파이프 파일 존재하지 않으면 생성하고 존재하면 생성하지 않음
    for (int i = 0; i < PLAYER_COUNT; i++) {
        if (access(player_in_pipes[i], F_OK) == -1) {
            if (mkfifo(player_in_pipes[i], 0666) == -1) {
                perror("파이프 생성 실패");
                exit(1);
            }
        }

        if (access(player_out_pipes[i], F_OK) == -1) {
            if (mkfifo(player_out_pipes[i], 0666) == -1) {
                perror("파이프 생성 실패");
                exit(1);
            }
        }
    }

    // 플레이어 연결 대기
    int connected_count = 0;
    printf("플레이어들의 입장을 기다리고 있습니다...\n");
    while (connected_count < PLAYER_COUNT) {
        for (int i = 0; i < PLAYER_COUNT; i++) {
            if (shm->player_connected[i] == 1 && shm->player_pids[i] != 0) {
                printf("플레이어 %d 입장 완료: PID=%d\n", i + 1, shm->player_pids[i]);
                connected_count++;
                shm->player_connected[i] = 2;  // 연결 확인 완료
            }
        }
        sleep(1);  // CPU 사용을 줄이기 위해 잠깐 대기
    }

    printf("모든 플레이어가 입장했습니다. 게임을 시작합니다...\n");

    // 각 플레이어와 통신하기 위한 파이프 열기
    for (int i = 0; i < PLAYER_COUNT; i++) {
        player_in_fds[i] = open(player_in_pipes[i], O_RDONLY);
        player_out_fds[i] = open(player_out_pipes[i], O_WRONLY);

        if (player_in_fds[i] == -1 || player_out_fds[i] == -1) {
            perror("파이프 열기 실패");
            exit(1);
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

    printf("플레이어 이름 입력받는중..\n");

    // 플레이어 초기화
    for (int i = 0; i < PLAYER_COUNT; i++) {
        sleep(1);
        snprintf(message, sizeof(message), "INPUT 플레이어 %d 이름을 입력하세요: (두 자로 해주세요.)", i + 1);
        write(player_out_fds[i], message, strlen(message) + 1);

        if (read(player_in_fds[i], buffer, sizeof(buffer)) > 0) {
            strcpy(players[i].name, buffer);
            sleep(1);
            snprintf(message, sizeof(message), "모든 플레이어가 입장할때까지 기다려 주세요..\n\n");
            write(player_out_fds[i], message, strlen(message) + 1);
        }


        players[i].money = 1000;  // 각 플레이어 초기 금액
        players[i].isActive = 1;  // 모든 플레이어는 처음에 활성 상태
        players[i].isAllIn = 0;
    }

    // 덱 초기화 및 셔플
    sleep(1);
    initializeDeck(deck);
    shuffleDeck(deck);

    // 게임 진행 루프
    while (countActivePlayers(players, PLAYER_COUNT) > 1) {
        // 1. 홀 카드 분배
        sleep(1);
        dealHoleCards(players, PLAYER_COUNT, deck, &deckIndex);

        sleep(1);
        for (int i = 0; i < PLAYER_COUNT; i++) {
            snprintf(message, sizeof(message), "당신이 현재 가지고 있는 금액: %d원\n\n", players[i].money);
            write(player_out_fds[i], message, strlen(message) + 1);
        }

        // 2. 베팅 라운드 (PREFLOP)
        printf("\n\n===================== 프리플롭 베팅 라운드 =====================\n\n");
        sleep(1);
        for (int i = 0; i < PLAYER_COUNT; i++) {
            snprintf(message, sizeof(message), "ROUND \n\n===================== 프리플롭 베팅 라운드 =====================\n\n차례를 기다려주세요.\n");
            write(player_out_fds[i], message, strlen(message) + 1);
        }
        sleep(1);
        startBettingRound(players, PLAYER_COUNT, &currentBet, &pot, &lastToRaiseIndex);

        // 만약 한 명의 플레이어만 남으면 바로 승리 처리
        sleep(1);
        Player* winner = checkForFoldWinner(players, PLAYER_COUNT);
        if (winner != NULL) {
            printf("\n%s님이 폴드하지 않고 남아있어 승리하셨습니다! 판돈 %d를 차지합니다!\n", winner->name, pot);
            for (int i = 0; i < PLAYER_COUNT; i++) {
                snprintf(message, sizeof(message), "\n%s님이 폴드하지 않고 남아있어 승리하셨습니다! 판돈 %d를 차지합니다!\n", winner->name, pot);
                write(player_out_fds[i], message, strlen(message) + 1);
            }
            winner->money += pot;
            continue;  // 게임 루프 재시작
        }

        // 3. 커뮤니티 카드 분배 및 각 라운드 진행
        for (Round currentRound = FLOP; currentRound <= RIVER; currentRound = (Round)((int)currentRound + 1)) {
            dealCommunityCards(communityCards, deck, &deckIndex, currentRound);
            sleep(1);
            switch (currentRound) {
            case FLOP:
                printf("\n\n===================== 플롭 베팅 라운드 =====================\n\n");
                for (int i = 0; i < PLAYER_COUNT; i++) {
                    snprintf(message, sizeof(message), "\n\n===================== 플롭 베팅 라운드 =====================\n\n차례를 기다려주세요.\n");
                    write(player_out_fds[i], message, strlen(message) + 1);
                }
                sleep(1);
                break;
            case TURN:
                printf("\n\n===================== 턴 베팅 라운드 =====================\n\n");
                for (int i = 0; i < PLAYER_COUNT; i++) {
                    snprintf(message, sizeof(message), "\n\n===================== 턴 베팅 라운드 =====================\n\n차례를 기다려주세요.\n");
                    write(player_out_fds[i], message, strlen(message) + 1);
                }
                sleep(1);
                break;
            case RIVER:
                printf("\n\n===================== 리버 베팅 라운드 =====================\n\n");
                for (int i = 0; i < PLAYER_COUNT; i++) {
                    snprintf(message, sizeof(message), "\n\n===================== 리버 베팅 라운드 =====================\n\n차례를 기다려주세요.\n");
                    write(player_out_fds[i], message, strlen(message) + 1);
                }
                sleep(1);
                break;
            default:
                break;
            }
            sleep(1);
            startBettingRound(players, PLAYER_COUNT, &currentBet, &pot, &lastToRaiseIndex);
            sleep(1);
            // 한 명의 플레이어만 남으면 바로 승리 처리
            winner = checkForFoldWinner(players, PLAYER_COUNT);
            sleep(1);
            if (winner != NULL) {
                printf("\n%s님이 폴드하지 않고 남아있어 승리하셨습니다! 판돈 %d를 차지합니다!\n", winner->name, pot);
                for (int i = 0; i < PLAYER_COUNT; i++) {
                    snprintf(message, sizeof(message), "\n%s님이 폴드하지 않고 남아있어 승리하셨습니다! 판돈 %d를 차지합니다!\n", winner->name, pot);
                    write(player_out_fds[i], message, strlen(message) + 1);
                }
                winner->money += pot;
                break;  // 커뮤니티 카드 라운드 루프 종료
            }
        }
        sleep(1);
        // 4. 승리자 판정
        if (winner == NULL) {
            determineWinners(players, PLAYER_COUNT, communityCards, &pot);
        }

        printf("\n");

        sleep(1);
        // 5. 게임 초기화
        resetGame(players, PLAYER_COUNT);
        currentBet = 0;
        deckIndex = 0;
        shuffleDeck(deck);  // 덱을 다시 셔플하여 새로운 게임 준비
    }
    sleep(1);
    // 최종 우승자 출력
    for (int i = 0; i < PLAYER_COUNT; i++) {
        if (players[i].isActive && players[i].money > 0) {
            printf("\n\n########################################################################\n\n\n		게임 종료! 최종 우승자는 %s입니다.\n\n\n########################################################################\n\n", players[i].name);
            for (int j = 0; j < PLAYER_COUNT; j++) {
                snprintf(message, sizeof(message), "\n\n########################################################################\n\n\n		게임 종료! 최종 우승자는 %s입니다.\n\n\n########################################################################\n\n", players[i].name);
                write(player_out_fds[j], message, strlen(message) + 1);
            }
            break;
        }
    }

    return 0;
}
