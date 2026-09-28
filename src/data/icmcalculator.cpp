#include "include/data/icmcalculator.h"

// Recursively assigns each paid place (0 = 1st) to every player in
// `remaining`, weighted by their share of the remaining chip pool at that
// point -- the standard Malmuth-Harville ICM model. Not memoized: for the
// realistic input size here (<=10 players, a final table), the full
// recursion is at most 10! calls in the extreme case of paying every
// place, comfortably fast, and memoizing on a player *set* would lose the
// place-order information the payouts depend on.
static void icmRecurse(const QVector<float>& stacks, QVector<int>& remaining,
                        const QVector<float>& payouts, int placeIndex,
                        float pathProb, QVector<float>& evOut){
    if(remaining.isEmpty() || placeIndex >= payouts.size()) return;

    float total = 0.0f;
    for(int idx : remaining) total += stacks[idx];
    if(total <= 0.0f) return;

    for(int i = 0; i < remaining.size(); i++){
        int idx = remaining[i];
        float p = stacks[idx] / total;
        evOut[idx] += pathProb * p * payouts[placeIndex];

        QVector<int> nextRemaining = remaining;
        nextRemaining.remove(i);
        icmRecurse(stacks, nextRemaining, payouts, placeIndex + 1, pathProb * p, evOut);
    }
}

QVector<float> icmEquity(const QVector<float>& stacks, const QVector<float>& payouts){
    QVector<float> evOut(stacks.size(), 0.0f);
    QVector<int> remaining;
    for(int i = 0; i < stacks.size(); i++) remaining.append(i);
    icmRecurse(stacks, remaining, payouts, 0, 1.0f, evOut);
    return evOut;
}

IcmCallVsFold compareCallVsFold(
    const QVector<float>& stacks, const QVector<float>& payouts,
    int heroIndex, int villainIndex, float equityHeroWins
){
    float evFold = icmEquity(stacks, payouts)[heroIndex];

    float combined = stacks[heroIndex] + stacks[villainIndex];

    QVector<float> stacksHeroWins = stacks;
    stacksHeroWins[heroIndex] = combined;
    stacksHeroWins[villainIndex] = 0.0f;
    float evIfWin = icmEquity(stacksHeroWins, payouts)[heroIndex];

    QVector<float> stacksHeroLoses = stacks;
    stacksHeroLoses[heroIndex] = 0.0f;
    stacksHeroLoses[villainIndex] = combined;
    float evIfLose = icmEquity(stacksHeroLoses, payouts)[heroIndex];

    IcmCallVsFold result;
    result.evFold = evFold;
    result.evCall = equityHeroWins * evIfWin + (1.0f - equityHeroWins) * evIfLose;
    return result;
}
