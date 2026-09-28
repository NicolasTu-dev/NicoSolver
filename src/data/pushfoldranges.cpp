#include "include/data/pushfoldranges.h"

// Range strings below are written fully expanded (no "+" shorthand): the
// app's range parser (PrivateRangeConverter::rangeStr2Cards) only
// understands explicit pair/suited/offsuit tokens like "QQ", "AKs", "AKo",
// not "QQ+" shorthand, and throws on anything else. Each string is also
// checked by hand to contain no duplicate hand token (the parser throws on
// duplicate combos too).

QString standardShoveRange(PushFoldScenario scenario, float effectiveStackBB){
    if(scenario == PushFoldScenario::OpenShove){
        if(effectiveStackBB <= 8.0f){
            return "22,33,44,55,66,77,88,99,TT,JJ,QQ,KK,AA,"
                   "A2,A3,A4,A5,A6,A7,A8,A9,AT,AJ,AQ,AK,"
                   "K5,K6,K7,K8,K9,KT,KJ,KQ,"
                   "T9s,98s,87s,76s,65s,54s";
        }
        if(effectiveStackBB <= 12.0f){
            return "22,33,44,55,66,77,88,99,TT,JJ,QQ,KK,AA,"
                   "A2,A3,A4,A5,A6,A7,A8,A9,AT,AJ,AQ,AK,"
                   "K8,K9,KT,KJ,KQ,"
                   "T9s,98s";
        }
        if(effectiveStackBB <= 16.0f){
            return "22,33,44,55,66,77,88,99,TT,JJ,QQ,KK,AA,"
                   "A7,A8,A9,AT,AJ,AQ,AK,"
                   "A2s,A3s,A4s,A5s,A6s,"
                   "KT,KJ,KQ";
        }
        if(effectiveStackBB <= 22.0f){
            return "22,33,44,55,66,77,88,99,TT,JJ,QQ,KK,AA,"
                   "A9,AT,AJ,AQ,AK,"
                   "A5s,A6s,A7s,A8s,"
                   "KJ,KQ";
        }
        return "55,66,77,88,99,TT,JJ,QQ,KK,AA,"
               "AJ,AQ,AK,"
               "ATs";
    }

    // ReshoveOverOpen: villain is 3-bet jamming over hero's raise, so the
    // range is meaningfully tighter than an OpenShove at the same depth.
    if(effectiveStackBB <= 10.0f){
        return "55,66,77,88,99,TT,JJ,QQ,KK,AA,"
               "A8,A9,AT,AJ,AQ,AK,"
               "A5s,A6s,A7s,"
               "KTs,KJs,KQ";
    }
    if(effectiveStackBB <= 15.0f){
        return "77,88,99,TT,JJ,QQ,KK,AA,"
               "AT,AJ,AQ,AK,"
               "KQs";
    }
    if(effectiveStackBB <= 20.0f){
        return "88,99,TT,JJ,QQ,KK,AA,"
               "AJs,AQs,AKs,AKo";
    }
    if(effectiveStackBB <= 25.0f){
        return "TT,JJ,QQ,KK,AA,"
               "AQs,AKs,AKo";
    }
    return "QQ,KK,AA,AKs,AKo";
}
