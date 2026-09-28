#include "include/data/icmcalculator.h"
#include <cstdio>
#include <cmath>

static int failures = 0;

static void expectNear(const char* name, float actual, float expected, float tolerance){
    if(std::fabs(actual - expected) > tolerance){
        printf("FAIL %s: got %.3f, expected %.3f +/- %.3f\n", name, actual, expected, tolerance);
        failures++;
    } else {
        printf("PASS %s: %.3f\n", name, actual);
    }
}

int main(){
    // 2 players, equal stacks, 1 paid place (winner take all): 50/50 split.
    {
        QVector<float> stacks = {50.0f, 50.0f};
        QVector<float> payouts = {100.0f};
        QVector<float> ev = icmEquity(stacks, payouts);
        expectNear("2p equal stacks winner-take-all [0]", ev[0], 50.0f, 0.01f);
        expectNear("2p equal stacks winner-take-all [1]", ev[1], 50.0f, 0.01f);
    }

    // 2 players, unequal stacks, 1 paid place: heads-up ICM with a single
    // payout is just a chip chop, EV proportional to stack.
    {
        QVector<float> stacks = {80.0f, 20.0f};
        QVector<float> payouts = {100.0f};
        QVector<float> ev = icmEquity(stacks, payouts);
        expectNear("2p 80/20 winner-take-all [0]", ev[0], 80.0f, 0.01f);
        expectNear("2p 80/20 winner-take-all [1]", ev[1], 20.0f, 0.01f);
    }

    // 3 players, equal stacks: by symmetry every player's $EV must be equal,
    // and must equal (sum of payouts)/3.
    {
        QVector<float> stacks = {100.0f, 100.0f, 100.0f};
        QVector<float> payouts = {500.0f, 300.0f, 200.0f};
        QVector<float> ev = icmEquity(stacks, payouts);
        float expected = (500.0f + 300.0f + 200.0f) / 3.0f;
        expectNear("3p equal stacks [0]", ev[0], expected, 0.5f);
        expectNear("3p equal stacks [1]", ev[1], expected, 0.5f);
        expectNear("3p equal stacks [2]", ev[2], expected, 0.5f);
    }

    // Invariant: total $EV distributed must equal total prize pool, for any
    // stacks/payouts where every input stack is positive.
    {
        QVector<float> stacks = {340.0f, 120.0f, 260.0f, 80.0f};
        QVector<float> payouts = {1000.0f, 600.0f, 350.0f};
        QVector<float> ev = icmEquity(stacks, payouts);
        float sumEv = ev[0] + ev[1] + ev[2] + ev[3];
        expectNear("4p sum(EV) == sum(payouts)", sumEv, 1000.0f + 600.0f + 350.0f, 0.5f);
    }

    // compareCallVsFold: heads-up, only 2 players left, 1 paid place, equal
    // stacks, hero has 100% equity if called -- calling must be worth
    // strictly more than folding (folding locks in the 50/50 chop; calling
    // with a lock to win takes 100% of the prize).
    {
        QVector<float> stacks = {50.0f, 50.0f};
        QVector<float> payouts = {100.0f};
        IcmCallVsFold result = compareCallVsFold(stacks, payouts, 0, 1, 1.0f);
        expectNear("call-vs-fold, 100% equity, evCall", result.evCall, 100.0f, 0.01f);
        expectNear("call-vs-fold, 100% equity, evFold", result.evFold, 50.0f, 0.01f);
    }

    if(failures > 0){
        printf("\n%d test(s) failed\n", failures);
        return 1;
    }
    printf("\nAll tests passed\n");
    return 0;
}
