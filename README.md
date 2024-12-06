gcc -o Pokerserver Pokerserver.c Card.c gamelogic.c
gcc -o player player.c

./Pokerserver    //메인서버

// 플레이어 클라이언트

./player 0    // 플레이어 1
./player 1    // 플레이어 2
./player 2    // 플레이어 3
./player 3    // 플레이어 4
