#ifndef QUICKMODERANGES_H
#define QUICKMODERANGES_H

#include <QString>
#include <QStringList>
#include <QVector>

struct QuickModeMatchup {
    QString descriptionAsOpener;  // shown when the user opened the pot
    QString descriptionAsCaller;  // shown when the user called
    QString openerRange;
    QString callerRange;
    bool openerIsIP;              // who acts later postflop
    float pot;
    float effectiveStack;
};

// Preflop acting order (first to act -> last to act) for a table size.
// 6 -> 6-max (UTG, HJ, CO, BTN, SB, BB). 9 -> Full Ring (adds UTG1, UTG2, LJ).
QStringList getTableSeatOrder(int tableSize);

// Human-readable label for a seat code ("UTG1" -> "UTG+1", etc).
QString getSeatDisplayName(QString seat);

// Builds ranges/pot/stack/position for an opener+caller pair at a given
// table size. Ranges are approximate, sourced from standard published
// 100bb opening-range charts (see conversation), not solved specifically
// for every position pair a reasonable approximation, not a precise GTO
// solve of that exact spot.
QuickModeMatchup buildQuickModeMatchup(int tableSize, QString openerSeat, QString callerSeat);

#endif // QUICKMODERANGES_H
