gcc -o PokerServer PokerServer.c Card.c gamelogic.c    // 메인서버 프로세스 실행파일

gcc -o player player.c    // 플레이어 프로세스 실행파일

./PokerServer    //메인서버

// 플레이어 클라이언트

./player 0    // 플레이어 1
./player 1    // 플레이어 2
./player 2    // 플레이어 3
./player 3    // 플레이어 4
