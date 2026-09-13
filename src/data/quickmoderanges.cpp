#include "include/data/quickmoderanges.h"
#include <QObject>
#include <QMap>

QStringList getTableSeatOrder(int tableSize) {
    if(tableSize == 9){
        return {"UTG","UTG1","UTG2","LJ","HJ","CO","BTN","SB","BB"};
    }
    return {"UTG","HJ","CO","BTN","SB","BB"};
}

static QStringList getPostflopOrder(int tableSize) {
    if(tableSize == 9){
        return {"SB","BB","UTG","UTG1","UTG2","LJ","HJ","CO","BTN"};
    }
    return {"SB","BB","UTG","HJ","CO","BTN"};
}

QString getSeatDisplayName(QString seat) {
    static QMap<QString,QString> names = {
        {"UTG", QObject::tr("UTG (Under the Gun)")},
        {"UTG1", QObject::tr("UTG+1")},
        {"UTG2", QObject::tr("UTG+2")},
        {"LJ", QObject::tr("Lojack (LJ)")},
        {"HJ", QObject::tr("Hijack (HJ)")},
        {"CO", QObject::tr("Cutoff (CO)")},
        {"BTN", QObject::tr("Button (BTN)")},
        {"SB", QObject::tr("Small Blind (SB)")},
        {"BB", QObject::tr("Big Blind (BB)")},
    };
    return names.value(seat, seat);
}

// Approximate 100bb opening ranges (raise-first-in), widening from UTG to
// BTN, standard for full ring / 6-max. Late positions (CO/BTN/SB) use the
// same range regardless of table size, since the number of players still to
// act behind them doesn't change.
static QString getOpenRange(int tableSize, QString seat) {
    static const QString utg6max = "22,33,44,55,66,77,88,99,TT,JJ,QQ,KK,AA,AJo,AQo,AKo,A5s,A9s,ATs,AJs,AQs,AKs,KTs,KJs,KQs,KQo,QTs,QJs,QJo,JTs,T9s,98s,87s,76s,65s";
    static const QString hj = "33,44,55,66,77,88,99,TT,JJ,QQ,KK,AA,A5s,A7s,A8s,A9s,ATs,AJs,AQs,AKs,AJo,ATo,AQo,AKo,K8s,K9s,KTs,KJs,KQs,KQo,Q9s,QTs,QJs,J9s,JTs,T9s";
    static const QString co = "22,33,44,55,66,77,88,99,TT,JJ,QQ,KK,AA,ATo,AJo,AQo,AKo,A2s,A3s,A4s,A5s,A6s,A7s,A8s,A9s,ATs,AJs,AQs,AKs,K9s,KTs,KJs,KQs,KTo:0.5,KQo,Q9s,QTs,QJs,QJo,J9s,JTs,T9s,T8s:0.5,98s,97s:0.5,87s,86s:0.5,76s,65s,54s:0.5";
    static const QString btn = "AA,KK,QQ,JJ,TT,99:0.75,88:0.75,77:0.5,66:0.25,55:0.25,AK,AQs,AQo:0.75,AJs,AJo:0.5,ATs:0.75,A6s:0.25,A5s:0.75,A4s:0.75,A3s:0.5,A2s:0.5,KQs,KQo:0.5,KJs,KTs:0.75,K5s:0.25,K4s:0.25,QJs:0.75,QTs:0.75,Q9s:0.5,JTs:0.75,J9s:0.75,J8s:0.75,T9s:0.75,T8s:0.75,T7s:0.75,98s:0.75,97s:0.75,96s:0.5,87s:0.75,86s:0.5,85s:0.5,76s:0.75,75s:0.5,65s:0.75,64s:0.5,54s:0.75,53s:0.5,43s:0.5";
    static const QString sb = "22,33,44,55,66,77,88,99,TT,JJ,QQ,KK,AA,A2o:0.5,A3o:0.5,A4o:0.5,A5o:0.5,A6o:0.5,A7o,A8o,A9o,ATo,AJo,AQo,AKo,A2s,A3s,A4s,A5s,A6s,A7s,A8s,A9s,ATs,AJs,AQs,AKs,K5s:0.5,K6s,K7s,K8s,K9s,KTs,KJs,KQs,KTo:0.5,KJo,KQo,Q7s:0.5,Q8s,Q9s,QTs,QJs,QTo:0.5,QJo,J8s,J9s,JTs,JTo:0.5,T8s,T9s,97s:0.5,98s,87s,76s,65s,54s,43s:0.5";
    static const QString utg9max = "77,88,99,TT,JJ,QQ,KK,AA,AJs,AQs,AKs,AQo,AKo,KQs";
    static const QString utg1 = "66,77,88,99,TT,JJ,QQ,KK,AA,ATs,AJs,AQs,AKs,AQo,AKo,KJs,KQs";
    static const QString utg2 = "55,66,77,88,99,TT,JJ,QQ,KK,AA,A9s,ATs,AJs,AQs,AKs,ATo,AQo,AKo,KTs,KJs,KQs,QJs";
    static const QString lj = "44,55,66,77,88,99,TT,JJ,QQ,KK,AA,A8s,A9s,ATs,AJs,AQs,AKs,ATo,AQo,AKo,K9s,KTs,KJs,KQs,KQo,QTs,QJs,JTs";

    if(seat == "UTG") return tableSize == 9 ? utg9max : utg6max;
    if(seat == "UTG1") return utg1;
    if(seat == "UTG2") return utg2;
    if(seat == "LJ") return lj;
    if(seat == "HJ") return hj;
    if(seat == "CO") return co;
    if(seat == "BTN") return btn;
    if(seat == "SB") return sb;
    return "";
}

// How wide BB defends depends on how loose the opener's range already is.
static QString getBBDefendRange(QString openerSeat) {
    static const QString vsTight = "22,33,44,55,66,77,88,99,TT,A9s,ATs,AJs,AQs,AQo:0.5,KQs,KJs,KQo:0.5,QJs,JTs,T9s,98s,87s,76s,65s";
    static const QString vsMedium = "22,33,44,55,66,77,88,99,TT,JJ:0.5,A8s,A9s,ATs,AJs,AQs,AQo:0.5,K9s,KTs,KJs,KQs,KQo:0.5,Q9s,QTs,QJs,J9s,JTs,T9s,T8s,98s,87s,86s:0.5,76s,65s,54s:0.5";
    static const QString vsWide = "QQ:0.5,JJ:0.75,TT,99,88,77,66,55,44,33,22,AKo:0.25,AQs,AQo:0.75,AJs,AJo:0.75,ATs,ATo:0.75,A9s,A8s,A7s,A6s,A5s,A4s,A3s,A2s,KQ,KJ,KTs,KTo:0.5,K9s,K8s,K7s,K6s,K5s,K4s:0.5,K3s:0.5,K2s:0.5,QJ,QTs,Q9s,Q8s,Q7s,JTs,JTo:0.5,J9s,J8s,T9s,T8s,T7s,98s,97s,96s,87s,86s,76s,75s,65s,64s,54s,53s,43s";
    static const QString vsSB = "22,33,44,55,66,77,88,99,TT,JJ,QQ:0.5,A2s,A3s,A4s,A5s,A6s,A7s,A8s,A9s,ATs,AJs,AQs:0.5,A8o:0.5,A9o,ATo,AJo,AQo:0.5,K7s:0.5,K8s,K9s,KTs,KJs,KQs,KTo:0.5,KJo,KQo:0.5,Q9s,QTs,QJs,QTo:0.5,J9s,JTs,JTo:0.5,T8s,T9s,98s,97s:0.5,87s,86s:0.5,76s,65s,54s,43s:0.5";

    if(openerSeat == "SB") return vsSB;
    if(openerSeat == "BTN") return vsWide;
    if(openerSeat == "HJ" || openerSeat == "CO") return vsMedium;
    return vsTight; // UTG, UTG1, UTG2, LJ
}

// A cold-call (flat) from a non-blind seat: tighter than a BB defend since
// there's no positional/price advantage, widened a bit if the open itself
// was already loose (late position).
static QString getFlatCallRange(QString openerSeat) {
    static const QString tight = "77,88,99,TT,JJ,QQ,KK,AJs,AQs,AKs,AQo,AKo,KQs,KJs,QJs,JTs,T9s,98s";
    static const QString wide = "66,77,88,99,TT,JJ,QQ,KK,A9s,ATs,AJs,AQs,AKs,AJo,AQo,AKo,KTs,KJs,KQs,KQo,QTs,QJs,JTs,T9s,98s";
    if(openerSeat == "CO" || openerSeat == "BTN" || openerSeat == "SB") return wide;
    return tight;
}

QuickModeMatchup buildQuickModeMatchup(int tableSize, QString openerSeat, QString callerSeat) {
    QStringList postflopOrder = getPostflopOrder(tableSize);
    int openerIdx = postflopOrder.indexOf(openerSeat);
    int callerIdx = postflopOrder.indexOf(callerSeat);

    QuickModeMatchup matchup;
    matchup.openerRange = getOpenRange(tableSize, openerSeat);
    matchup.callerRange = (callerSeat == "BB") ? getBBDefendRange(openerSeat) : getFlatCallRange(openerSeat);
    matchup.openerIsIP = openerIdx > callerIdx;
    matchup.pot = 50.0f;
    matchup.effectiveStack = 200.0f;
    matchup.descriptionAsOpener = QObject::tr("You opened from %1 and your opponent called from %2")
        .arg(getSeatDisplayName(openerSeat), getSeatDisplayName(callerSeat));
    matchup.descriptionAsCaller = QObject::tr("Your opponent opened from %1 and you called from %2")
        .arg(getSeatDisplayName(openerSeat), getSeatDisplayName(callerSeat));
    return matchup;
}
