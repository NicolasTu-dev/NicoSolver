#ifndef PREFLOPEQUITY_H
#define PREFLOPEQUITY_H

#include <QVector>
#include <QString>
#include <memory>
#include "include/compairer/Compairer.h"

// Preflop hand-vs-range equity, used by the ICM push/fold feature to know
// how often heroCards beats a random hand from villainRange. Reuses the
// same 5-card hand evaluator (Compairer) the CFR engine uses for postflop
// showdowns. Monte Carlo samples random runouts rather than enumerating
// every board, so it stays fast regardless of how wide villainRange is.

// heroCards: exactly 2 card strings, e.g. {"Jh","Jd"}. villainRange: a
// range string in the format used elsewhere in the app (e.g.
// "QQ+,AKs,AKo"). compairer: an already-constructed hand evaluator --
// callers should reuse the app's existing one (e.g.
// mainWindow->qSolverJob->ps_holdem.getCompairer()) instead of building a
// new one, since loading the dictionary file takes several seconds.
// Returns hero's equity as a value in [0,1]. Returns 0.5 if heroCards
// isn't exactly 2 cards, villainRange is empty, or villainRange has no
// combos left once heroCards' cards are removed from it.
float handVsRangeEquity(const QVector<QString>& heroCards, const QString& villainRange,
                         std::shared_ptr<Compairer> compairer);

#endif // PREFLOPEQUITY_H
