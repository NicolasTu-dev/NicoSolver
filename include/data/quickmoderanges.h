#ifndef QUICKMODERANGES_H
#define QUICKMODERANGES_H

#include <QString>
#include <QVector>

struct QuickModeMatchup {
    QString id;
    QString descriptionAsOpener;  // shown when the user opened the pot
    QString descriptionAsCaller;  // shown when the user called (or called a 3-bet)
    QString openerRange;
    QString callerRange;
    bool openerIsIP;              // false only for SB-opens-BB-calls (BB acts last postflop)
    float pot;
    float effectiveStack;
};

QVector<QuickModeMatchup> getQuickModeMatchups();

#endif // QUICKMODERANGES_H
