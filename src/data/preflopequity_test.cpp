#include "include/data/preflopequity.h"
#include "include/compairer/Dic5Compairer.h"
#include <cstdio>
#include <memory>

static int failures = 0;

static void expectRange(const char* name, float actual, float low, float high){
    if(actual < low || actual > high){
        printf("FAIL %s: got %.3f, expected [%.3f, %.3f]\n", name, actual, low, high);
        failures++;
    } else {
        printf("PASS %s: %.3f\n", name, actual);
    }
}

int main(){
    printf("Loading hand evaluator dictionary (a few seconds)...\n");
    auto compairer = std::make_shared<Dic5Compairer>(
        "resources/compairer/card5_dic_sorted.txt", 2598961,
        "resources/compairer/card5_dic_zipped.bin");
    printf("Loaded.\n");

    // JJ is a clear underdog to a QQ,KK,AA only range (dominated by 3 overpairs).
    float eq1 = handVsRangeEquity({"Jh","Jd"}, "QQ,KK,AA", compairer);
    expectRange("JJ vs QQ+", eq1, 0.15f, 0.25f);

    // JJ is a clear favorite against a wide short-stack shove range: it
    // dominates most pairs/broadways in the range and only trails the top.
    float eq2 = handVsRangeEquity({"Jh","Jd"},
        "22,33,44,55,66,77,88,99,TT,JJ,QQ,KK,AA,A2s,A3s,A4s,A5s,A6s,A7s,A8s,A9s,ATs,AJs,AQs,AKs,"
        "K9s,KTs,KJs,KQs,QTs,JTs,A2o,A3o,A4o,A5o,A6o,A7o,A8o,A9o,ATo,AJo,AQo,AKo,K9o,KTo,KJo,KQo,QTo",
        compairer);
    expectRange("JJ vs wide shove range", eq2, 0.50f, 0.70f);

    // Card-blocking sanity check: hero holds 2 of the 4 aces, so a
    // villain range of "AA" only has 1 combo left (the other 2 aces).
    // Both hands are "a pair of aces" -- on a shared board they tie in
    // the overwhelming majority of runouts, so equity should land close
    // to 50% (a tie counts as 0.5 equity to each side).
    float eq3 = handVsRangeEquity({"As","Ad"}, "AA", compairer);
    expectRange("AA vs AA range (blocked down to 1 combo)", eq3, 0.45f, 0.55f);

    if(failures > 0){
        printf("\n%d test(s) failed\n", failures);
        return 1;
    }
    printf("\nAll tests passed\n");
    return 0;
}
