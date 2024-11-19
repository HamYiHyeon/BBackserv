#ifndef CARD_H
#define CARD_H

#define DECK_SIZE 52    // ���� �� ī�� ��

/* �÷��̾��� ī��� Ŀ�´�Ƽ ī�带 �����Ͽ� �÷��̾��� �и� �����ϴ� �����Դϴ�. */

// ī�� ����ü ����
typedef struct {
    int suit;  // ī���� ���� (0: Hearts, 1: Diamonds, 2: Clubs, 3: Spades)
    int rank;  // ī���� �� (2-14, 11: Jack, 12: Queen, 13: King, 14: Ace)
} Card;

// �ڵ� ��ŷ ����
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
    HandRank rank;     // �ڵ��� ���� (��: �����, �÷��� ��)
    int highCard;      // ���� ���� ī���� ����
    int kicker[5];     // kicker ī��� (�ڵ� ��ũ�� ���� ��� �񱳿� ���)
} HandEvaluation;

HandEvaluation evaluateHand(Card playerCards[], Card communityCards[]);
void getCombinations(Card cards[], int numCards, Card combination[], int index, int start, int comboSize, HandEvaluation* bestEvaluation);
HandEvaluation evaluateFiveCardHand(Card cards[]);
int getHighestRank(Card cards[], int cardCount);


#endif