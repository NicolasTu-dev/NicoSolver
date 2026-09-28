#include "include/data/pushfoldranges.h"
#include "include/tools/PrivateRangeConverter.h"
#include <cstdio>

static int failures = 0;

static void expectTrue(const char* name, bool condition){
    if(!condition){
        printf("FAIL %s\n", name);
        failures++;
    } else {
        printf("PASS %s\n", name);
    }
}

static int comboCount(const QString& range){
    // rangeStr2Cards requires a non-empty blocker vector (Card::boardInts2long
    // throws on an empty board). Card index 0 ("2c") is an arbitrary, valid
    // placeholder -- it only filters combos that literally use that one
    // card, which affects every call equally and doesn't change any of the
    // relative comparisons this test makes.
    vector<int> placeholderBlocker = {0};
    return (int)PrivateRangeConverter::rangeStr2Cards(range.toStdString(), placeholderBlocker).size();
}

int main(){
    // Every band must parse into a non-empty, valid range string.
    float depths[] = {5.0f, 9.0f, 14.0f, 20.0f, 30.0f};
    for(float d : depths){
        expectTrue("OpenShove parses and non-empty at depth",
                   comboCount(standardShoveRange(PushFoldScenario::OpenShove, d)) > 0);
        expectTrue("ReshoveOverOpen parses and non-empty at depth",
                   comboCount(standardShoveRange(PushFoldScenario::ReshoveOverOpen, d)) > 0);
    }

    // Shorter effective stack must never produce a *narrower* OpenShove
    // range than a deeper stack (push ranges only get tighter as stacks
    // grow, never the reverse).
    int wide = comboCount(standardShoveRange(PushFoldScenario::OpenShove, 5.0f));
    int narrow = comboCount(standardShoveRange(PushFoldScenario::OpenShove, 30.0f));
    expectTrue("OpenShove range shrinks as stack grows", wide >= narrow);

    // At the same depth, a 3-bet jam range must never be wider than an
    // open-shove range (re-shoving over an open is always at least as
    // tight).
    int openAt15 = comboCount(standardShoveRange(PushFoldScenario::OpenShove, 15.0f));
    int reshoveAt15 = comboCount(standardShoveRange(PushFoldScenario::ReshoveOverOpen, 15.0f));
    expectTrue("ReshoveOverOpen <= OpenShove at same depth", reshoveAt15 <= openAt15);

    // Out-of-band depths clamp instead of crashing/returning empty.
    expectTrue("very short depth clamps", comboCount(standardShoveRange(PushFoldScenario::OpenShove, 0.5f)) > 0);
    expectTrue("very deep depth clamps", comboCount(standardShoveRange(PushFoldScenario::OpenShove, 100.0f)) > 0);

    if(failures > 0){
        printf("\n%d test(s) failed\n", failures);
        return 1;
    }
    printf("\nAll tests passed\n");
    return 0;
}
