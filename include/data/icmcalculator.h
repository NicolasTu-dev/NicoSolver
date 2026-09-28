#ifndef ICMCALCULATOR_H
#define ICMCALCULATOR_H

#include <QVector>

// Independent Chip Model (ICM): converts tournament chip stacks into real
// dollar equity given a payout structure. This is what actually determines
// whether a call is correct at a paid final table -- raw chip EV is not
// enough once pay jumps are in play.

// stacks: chip count of each player, any order (the order defines the
// index used to read the result). payouts: dollar payout of each paid
// place, payouts[0] = 1st place, payouts[1] = 2nd place, etc. Places
// beyond payouts.size() are treated as $0 (not paid). Returns the $EV of
// each player, same order/index as `stacks`.
QVector<float> icmEquity(const QVector<float>& stacks, const QVector<float>& payouts);

struct IcmCallVsFold {
    float evCall;
    float evFold;
};

// heroIndex/villainIndex: indices into `stacks`. equityHeroWins: hero's
// preflop equity (0..1) against villain's assumed range, from
// preflopequity.h. Compares hero's $EV if they call the all-in (weighting
// the "hero wins the pot" and "hero busts" outcomes by equityHeroWins)
// against hero's $EV if they fold (stacks unchanged).
IcmCallVsFold compareCallVsFold(
    const QVector<float>& stacks, const QVector<float>& payouts,
    int heroIndex, int villainIndex, float equityHeroWins
);

#endif // ICMCALCULATOR_H
