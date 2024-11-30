#include "Card.h"
#include <stdlib.h>
#include <stdio.h>
#include "gamelogic.h"

// 핸드를 평가하는 함수
HandEvaluation evaluateHand(Card playerCards[], Card communityCards[]) {
    HandEvaluation bestEvaluation = { HIGH_CARD, 0, {0} };

    // 총 7장의 카드 중에서 가장 좋은 5장의 조합을 평가
    Card allCards[7];
    allCards[0] = playerCards[0];
    allCards[1] = playerCards[1];
    for (int i = 0; i < 5; i++) {
        allCards[2 + i] = communityCards[i];
    }

    // 7장의 카드 중에서 5장을 선택하는 모든 조합을 평가
    Card combination[5];
    getCombinations(allCards, 7, combination, 0, 0, 5, &bestEvaluation);

    return bestEvaluation;
}

void getCombinations(Card cards[], int numCards, Card combination[], int index, int start, int comboSize, HandEvaluation* bestEvaluation) {
    // 7장에서 5장을 선택하는 조합을 재귀적으로 구함
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

    // 카드의 랭크와 모양을 정렬하고, 핸드를 분석하여 랭크 평가
    int rankCounts[15] = { 0 };  // 카드 랭크 카운트 (Ace는 14로 설정)
    int suitCounts[4] = { 0 };   // 카드 슈트 카운트

    // 각 카드의 랭크와 모양을 카운트
    for (int i = 0; i < 5; i++) {
        rankCounts[cards[i].rank]++;
        suitCounts[cards[i].suit]++;
    }

    // 플러시 여부 판단
    int isFlush = 0;
    for (int i = 0; i < 4; i++) {
        if (suitCounts[i] == 5) {
            isFlush = 1;
            break;
        }
    }

    // 스트레이트 여부 판단
    int isStraight = 0;
    int highCard = 0;
    for (int i = 14; i >= 5; i--) {
        if (rankCounts[i] == 1 && rankCounts[i - 1] == 1 && rankCounts[i - 2] == 1 &&
            rankCounts[i - 3] == 1 && rankCounts[i - 4] == 1) {
            isStraight = 1;
            highCard = i;
            break;
        }
    }

    // 특수 스트레이트 (A, 2, 3, 4, 5) 여부 확인
    if (!isStraight && rankCounts[14] && rankCounts[2] && rankCounts[3] && rankCounts[4] && rankCounts[5]) {
        isStraight = 1;
        highCard = 5;
    }

    // 핸드 랭크 결정
    if (isFlush && isStraight) {
        evaluation.rank = (highCard == 14) ? ROYAL_FLUSH : STRAIGHT_FLUSH;
        evaluation.highCard = highCard;
    }
    else if (isFlush) {
        evaluation.rank = FLUSH;
        evaluation.highCard = getHighestRank(cards, 5);
        setKickers(cards, 5, evaluation.kicker, evaluation.highCard);
    }
    else if (isStraight) {
        evaluation.rank = STRAIGHT;
        evaluation.highCard = highCard;
        setKickers(cards, 5, evaluation.kicker, evaluation.highCard);
    }
    else {
        // 페어나 트리플 등 나머지 핸드 평가
        int fourOfAKind = 0, threeOfAKind = 0, pairs = 0;
        int kickerIndex = 0;

        for (int i = 14; i >= 2; i--) {
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
            setKickers(cards, 5, evaluation.kicker, evaluation.highCard);
        }
    }

    return evaluation;
}

void setKickers(Card cards[], int cardCount, int kicker[], int highCard) {
    // 카드를 내림차순으로 정렬하여 키커 설정
    Card sortedCards[5];
    int sortedIndex = 0;

    // 먼저 highCard를 제외한 나머지 카드를 저장합니다.
    for (int i = 0; i < cardCount; i++) {
        if (cards[i].rank != highCard || sortedIndex >= 4) {
            sortedCards[sortedIndex++] = cards[i];
        }
    }

    // 나머지 카드를 정렬합니다.
    for (int i = 0; i < sortedIndex - 1; i++) {
        for (int j = i + 1; j < sortedIndex; j++) {
            if (sortedCards[j].rank > sortedCards[i].rank) {
                Card temp = sortedCards[i];
                sortedCards[i] = sortedCards[j];
                sortedCards[j] = temp;
            }
        }
    }

    // 정렬된 카드를 kicker에 저장합니다.
    for (int i = 0; i < sortedIndex; i++) {
        kicker[i] = sortedCards[i].rank;
    }
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
