#ifndef CARD_H
#define CARD_H

#define DECK_SIZE 52    // 덱의 총 카드 수

/* 플레이어의 카드와 커뮤니티 카드를 조합하여 플레이어의 패를 결정하는 로직입니다. */

// 카드 구조체 정의
typedef struct {
    int suit;  // 카드의 문양 (0: Hearts, 1: Diamonds, 2: Clubs, 3: Spades)
    int rank;  // 카드의 값 (2-14, 11: Jack, 12: Queen, 13: King, 14: Ace)
} Card;

// 핸드 랭킹 순위
typedef enum {
    HIGH_CARD,
    ONE_PAIR,
    TWO_PAIR,
    THREE_OF_A_KIND,
    STRAIGHT,
    FLUSH,
    FULL_HOUSE,
    FOUR_OF_A_KIND,
    STRAIGHT_FLUSH,
    ROYAL_FLUSH
} HandRank;

typedef struct {
    HandRank rank;     // 핸드의 종류 (예: 원페어, 플러시 등)
    int highCard;      // 가장 높은 카드의 순위
    int kicker[5];     // kicker 카드들 (핸드 랭크가 같을 경우 비교에 사용)
} HandEvaluation;

HandEvaluation evaluateHand(Card playerCards[], Card communityCards[]);
void getCombinations(Card cards[], int numCards, Card combination[], int index, int start, int comboSize, HandEvaluation* bestEvaluation);
HandEvaluation evaluateFiveCardHand(Card cards[]);
int getHighestRank(Card cards[], int cardCount);


#endif