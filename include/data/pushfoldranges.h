#ifndef PUSHFOLDRANGES_H
#define PUSHFOLDRANGES_H

#include <QString>

// Standard approximate push/fold ranges by stack depth, used as the
// assumed villain range for the ICM push/fold feature. These are
// approximate, general-purpose charts, not a solve for every exact
// stack/scenario -- same spirit as the ranges in quickmoderanges.h.

enum class PushFoldScenario {
    OpenShove,       // villain's all-in is their first voluntary action (no open before it)
    ReshoveOverOpen  // villain shoves over hero's open (a 3-bet jam), tighter than OpenShove
};

// effectiveStackBB: the shoving player's effective stack depth, in big
// blinds. Values outside the covered bands are clamped to the nearest
// band. Returns a range string in the format used elsewhere in the app
// (e.g. "QQ,KK,AA,AKs,AKo").
QString standardShoveRange(PushFoldScenario scenario, float effectiveStackBB);

#endif // PUSHFOLDRANGES_H
