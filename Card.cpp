#include "Card.h"
#include <stdlib.h>
#include <stdio.h>
#include "gamelogic.h"

// �ڵ带 ���ϴ� �Լ�
HandEvaluation evaluateHand(Card playerCards[], Card communityCards[]) {
    HandEvaluation bestEvaluation = { HIGH_CARD, 0, {0} };

    // �� 7���� ī�� �߿��� ���� ���� 5���� ������ ��
    Card allCards[7];
    allCards[0] = playerCards[0];
    allCards[1] = playerCards[1];
    for (int i = 0; i < 5; i++) {
        allCards[2 + i] = communityCards[i];
    }

    // 7���� ī�� �߿��� 5���� �����ϴ� ��� ������ ��
    Card combination[5];
    getCombinations(allCards, 7, combination, 0, 0, 5, &bestEvaluation);

    return bestEvaluation;
}

void getCombinations(Card cards[], int numCards, Card combination[], int index, int start, int comboSize, HandEvaluation* bestEvaluation) {
    // 7�忡�� 5���� �����ϴ� ������ ��������� ����
    if (index == comboSize) {
        HandEvaluation eval = evaluateFiveCardHand(combination);
        if (eval.rank > bestEvaluation->rank ||
            (eval.rank == bestEvaluation->rank && eval.highCard > bestEvaluation->highCard) ||
            (eval.rank == bestEvaluation->rank && eval.highCard == bestEvaluation->highCard &&
                compareKickers(eval.kicker, bestEvaluation->kicker) > 0)) {
            *bestEvaluation = eval;
        }
        return;
    }

    for (int i = start; i < numCards; i++) {
        combination[index] = cards[i];
        getCombinations(cards, numCards, combination, index + 1, i + 1, comboSize, bestEvaluation);
    }
}

HandEvaluation evaluateFiveCardHand(Card cards[]) {
    HandEvaluation evaluation = { HIGH_CARD, 0, {0} };

    // ī���� ��ũ�� ����� �����ϰ�, �ڵ带 �м��Ͽ� ��ũ ��
    int rankCounts[15] = { 0 };  // ī�� ��ũ ī��Ʈ (Ace�� 14�� ����)
    int suitCounts[4] = { 0 };   // ī�� ��Ʈ ī��Ʈ

    // �� ī���� ��ũ�� ����� ī��Ʈ
    for (int i = 0; i < 5; i++) {
        rankCounts[cards[i].rank]++;
        suitCounts[cards[i].suit]++;
    }

    // �÷��� ���� �Ǵ�
    int isFlush = 0;
    for (int i = 0; i < 4; i++) {
        if (suitCounts[i] == 5) {
            isFlush = 1;
            break;
        }
    }

    // ��Ʈ����Ʈ ���� �Ǵ�
    int isStraight = 0;
    int highCard = 0;
    for (int i = 2; i <= 10; i++) {
        if (rankCounts[i] == 1 && rankCounts[i + 1] == 1 && rankCounts[i + 2] == 1 &&
            rankCounts[i + 3] == 1 && rankCounts[i + 4] == 1) {
            isStraight = 1;
            highCard = i + 4;
            break;
        }
    }

    // Ư�� ��Ʈ����Ʈ (A, 2, 3, 4, 5) ���� Ȯ��
    if (!isStraight && rankCounts[14] && rankCounts[2] && rankCounts[3] && rankCounts[4] && rankCounts[5]) {
        isStraight = 1;
        highCard = 5;
    }

    // �ڵ� ��ũ ����
    if (isFlush && isStraight) {
        evaluation.rank = (highCard == 14) ? ROYAL_FLUSH : STRAIGHT_FLUSH;
        evaluation.highCard = highCard;
    }
    else if (isFlush) {
        evaluation.rank = FLUSH;
        evaluation.highCard = getHighestRank(cards, 5);
    }
    else if (isStraight) {
        evaluation.rank = STRAIGHT;
        evaluation.highCard = highCard;
    }
    else {
        // �� Ʈ���� �� ������ �ڵ� ��
        int fourOfAKind = 0, threeOfAKind = 0, pairs = 0;
        int kickerIndex = 0;

        for (int i = 2; i <= 14; i++) {
            if (rankCounts[i] == 4) {
                fourOfAKind = i;
            }
            else if (rankCounts[i] == 3) {
                threeOfAKind = i;
            }
            else if (rankCounts[i] == 2) {
                pairs++;
            }
        }

        if (fourOfAKind) {
            evaluation.rank = FOUR_OF_A_KIND;
            evaluation.highCard = fourOfAKind;
            for (int i = 14; i >= 2; i--) {
                if (rankCounts[i] == 1) {
                    evaluation.kicker[kickerIndex++] = i;
                    break;
                }
            }
        }
        else if (threeOfAKind && pairs) {
            evaluation.rank = FULL_HOUSE;
            evaluation.highCard = threeOfAKind;
        }
        else if (threeOfAKind) {
            evaluation.rank = THREE_OF_A_KIND;
            evaluation.highCard = threeOfAKind;
            for (int i = 14; i >= 2; i--) {
                if (rankCounts[i] == 1) {
                    evaluation.kicker[kickerIndex++] = i;
                }
            }
        }
        else if (pairs == 2) {
            evaluation.rank = TWO_PAIR;
            int highPair = 0, lowPair = 0;
            for (int i = 14; i >= 2; i--) {
                if (rankCounts[i] == 2) {
                    if (highPair == 0) {
                        highPair = i;
                    }
                    else {
                        lowPair = i;
                    }
                }
            }
            evaluation.highCard = highPair;
            evaluation.kicker[kickerIndex++] = lowPair;
            for (int i = 14; i >= 2; i--) {
                if (rankCounts[i] == 1) {
                    evaluation.kicker[kickerIndex++] = i;
                    break;
                }
            }
        }
        else if (pairs == 1) {
            evaluation.rank = ONE_PAIR;
            for (int i = 14; i >= 2; i--) {
                if (rankCounts[i] == 2) {
                    evaluation.highCard = i;
                    break;
                }
            }
            for (int i = 14; i >= 2; i--) {
                if (rankCounts[i] == 1) {
                    evaluation.kicker[kickerIndex++] = i;
                }
            }
        }
        else {
            evaluation.rank = HIGH_CARD;
            evaluation.highCard = getHighestRank(cards, 5);

            for (int i = 14; i >= 2; i--) {
                if (rankCounts[i] == 1) {
                    evaluation.kicker[kickerIndex++] = i;
                }
            }
        }
    }

    return evaluation;
}

int getHighestRank(Card cards[], int cardCount) {
    int highestRank = 0;
    for (int i = 0; i < cardCount; i++) {
        if (cards[i].rank > highestRank) {
            highestRank = cards[i].rank;
        }
    }
    return highestRank;
}
