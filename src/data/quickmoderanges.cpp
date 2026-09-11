#include "include/data/quickmoderanges.h"
#include <QObject>

QVector<QuickModeMatchup> getQuickModeMatchups() {
    QVector<QuickModeMatchup> matchups;

    matchups.append({
        "utg_open_bb_call",
        QObject::tr("Abriste desde UTG y el rival pagó desde la ciega grande (BB)"),
        QObject::tr("El rival abrió desde UTG y vos pagaste desde la ciega grande (BB)"),
        "22,33,44,55,66,77,88,99,TT,JJ,QQ,KK,AA,AJo,AQo,AKo,A5s,A9s,ATs,AJs,AQs,AKs,KTs,KJs,KQs,KQo,QTs,QJs,QJo,JTs,T9s,98s,87s,76s,65s",
        "22,33,44,55,66,77,88,99,TT,A9s,ATs,AJs,AQs,AQo:0.5,KQs,KJs,KQo:0.5,QJs,JTs,T9s,98s,87s,76s,65s",
        true, 50.0f, 200.0f
    });

    matchups.append({
        "co_open_bb_call",
        QObject::tr("Abriste desde el Cutoff (CO) y el rival pagó desde la ciega grande (BB)"),
        QObject::tr("El rival abrió desde el Cutoff (CO) y vos pagaste desde la ciega grande (BB)"),
        "22,33,44,55,66,77,88,99,TT,JJ,QQ,KK,AA,ATo,AJo,AQo,AKo,A2s,A3s,A4s,A5s,A6s,A7s,A8s,A9s,ATs,AJs,AQs,AKs,K9s,KTs,KJs,KQs,KTo:0.5,KQo,Q9s,QTs,QJs,QJo,J9s,JTs,T9s,T8s:0.5,98s,97s:0.5,87s,86s:0.5,76s,65s,54s:0.5",
        "22,33,44,55,66,77,88,99,TT,JJ:0.5,A8s,A9s,ATs,AJs,AQs,AQo:0.5,K9s,KTs,KJs,KQs,KQo:0.5,Q9s,QTs,QJs,J9s,JTs,T9s,T8s,98s,87s,86s:0.5,76s,65s,54s:0.5",
        true, 50.0f, 200.0f
    });

    matchups.append({
        "btn_open_bb_call",
        QObject::tr("Abriste desde el Botón (BTN) y el rival pagó desde la ciega grande (BB)"),
        QObject::tr("El rival abrió desde el Botón (BTN) y vos pagaste desde la ciega grande (BB)"),
        "AA,KK,QQ,JJ,TT,99:0.75,88:0.75,77:0.5,66:0.25,55:0.25,AK,AQs,AQo:0.75,AJs,AJo:0.5,ATs:0.75,A6s:0.25,A5s:0.75,A4s:0.75,A3s:0.5,A2s:0.5,KQs,KQo:0.5,KJs,KTs:0.75,K5s:0.25,K4s:0.25,QJs:0.75,QTs:0.75,Q9s:0.5,JTs:0.75,J9s:0.75,J8s:0.75,T9s:0.75,T8s:0.75,T7s:0.75,98s:0.75,97s:0.75,96s:0.5,87s:0.75,86s:0.5,85s:0.5,76s:0.75,75s:0.5,65s:0.75,64s:0.5,54s:0.75,53s:0.5,43s:0.5",
        "QQ:0.5,JJ:0.75,TT,99,88,77,66,55,44,33,22,AKo:0.25,AQs,AQo:0.75,AJs,AJo:0.75,ATs,ATo:0.75,A9s,A8s,A7s,A6s,A5s,A4s,A3s,A2s,KQ,KJ,KTs,KTo:0.5,K9s,K8s,K7s,K6s,K5s,K4s:0.5,K3s:0.5,K2s:0.5,QJ,QTs,Q9s,Q8s,Q7s,JTs,JTo:0.5,J9s,J8s,T9s,T8s,T7s,98s,97s,96s,87s,86s,76s,75s,65s,64s,54s,53s,43s",
        true, 50.0f, 200.0f
    });

    matchups.append({
        "sb_open_bb_call",
        QObject::tr("Abriste desde la ciega chica (SB) y el rival pagó desde la ciega grande (BB)"),
        QObject::tr("El rival abrió desde la ciega chica (SB) y vos pagaste desde la ciega grande (BB)"),
        "22,33,44,55,66,77,88,99,TT,JJ,QQ,KK,AA,A2o:0.5,A3o:0.5,A4o:0.5,A5o:0.5,A6o:0.5,A7o,A8o,A9o,ATo,AJo,AQo,AKo,A2s,A3s,A4s,A5s,A6s,A7s,A8s,A9s,ATs,AJs,AQs,AKs,K5s:0.5,K6s,K7s,K8s,K9s,KTs,KJs,KQs,KTo:0.5,KJo,KQo,Q7s:0.5,Q8s,Q9s,QTs,QJs,QTo:0.5,QJo,J8s,J9s,JTs,JTo:0.5,T8s,T9s,97s:0.5,98s,87s,76s,65s,54s,43s:0.5",
        "22,33,44,55,66,77,88,99,TT,JJ,QQ:0.5,A2s,A3s,A4s,A5s,A6s,A7s,A8s,A9s,ATs,AJs,AQs:0.5,A8o:0.5,A9o,ATo,AJo,AQo:0.5,K7s:0.5,K8s,K9s,KTs,KJs,KQs,KTo:0.5,KJo,KQo:0.5,Q9s,QTs,QJs,QTo:0.5,J9s,JTs,JTo:0.5,T8s,T9s,98s,97s:0.5,87s,86s:0.5,76s,65s,54s,43s:0.5",
        false, 60.0f, 195.0f
    });

    matchups.append({
        "btn_open_bb_3bet_btn_call",
        QObject::tr("Abriste desde el Botón (BTN), el rival subió (3-bet) desde la ciega grande y vos pagaste"),
        QObject::tr("El rival abrió desde el Botón (BTN), subiste (3-bet) desde la ciega grande y el rival pagó"),
        "QQ,KK,AA:0.5,JJ:0.5,TT:0.5,AQs,AKs,AKo:0.5,AJs:0.5,KQs:0.5,ATs:0.3",
        "AA,KK,QQ,AKs,AKo,A5s,A4s,JJ:0.5,TT:0.3,KQs:0.3,A2s:0.3",
        true, 130.0f, 175.0f
    });

    matchups.append({
        "co_open_btn_3bet_co_call",
        QObject::tr("Abriste desde el Cutoff (CO), el rival subió (3-bet) desde el Botón y vos pagaste"),
        QObject::tr("El rival abrió desde el Cutoff (CO), subiste (3-bet) desde el Botón y el rival pagó"),
        "QQ,KK,AA:0.5,JJ:0.5,TT:0.4,AQs,AKs,AKo:0.5,AJs:0.3",
        "AA,KK,QQ,AKs,AKo,A5s,A4s,JJ:0.5,KQs:0.3,A2s:0.3",
        false, 130.0f, 175.0f
    });

    return matchups;
}
