#include "include/data/preflopequity.h"
#include "include/Card.h"
#include "include/Deck.h"
#include "include/tools/PrivateRangeConverter.h"
#include "include/ranges/PrivateCards.h"
#include <random>
#include <algorithm>

static const int SAMPLES_PER_COMBO = 300;

float handVsRangeEquity(const QVector<QString>& heroCards, const QString& villainRange,
                         std::shared_ptr<Compairer> compairer){
    if(heroCards.size() != 2 || villainRange.trimmed().isEmpty()) return 0.5f;

    vector<int> heroInts = {
        Card::strCard2int(heroCards[0].toStdString()),
        Card::strCard2int(heroCards[1].toStdString())
    };

    vector<PrivateCards> villainCombos = PrivateRangeConverter::rangeStr2Cards(
        villainRange.toStdString(), heroInts);
    if(villainCombos.empty()) return 0.5f;

    Deck deck({"2","3","4","5","6","7","8","9","T","J","Q","K","A"}, {"c","d","h","s"});
    vector<Card>& fullDeck = deck.getCards();

    std::mt19937 rng(std::random_device{}());

    double totalWeight = 0.0;
    double wins = 0.0;
    double ties = 0.0;

    for(const PrivateCards& villain : villainCombos){
        vector<int> usedCards = { heroInts[0], heroInts[1], villain.card1, villain.card2 };
        vector<int> remaining;
        for(Card& c : fullDeck){
            int cardInt = c.getCardInt();
            if(find(usedCards.begin(), usedCards.end(), cardInt) == usedCards.end()){
                remaining.push_back(cardInt);
            }
        }

        for(int sample = 0; sample < SAMPLES_PER_COMBO; sample++){
            std::shuffle(remaining.begin(), remaining.end(), rng);
            vector<int> board(remaining.begin(), remaining.begin() + 5);

            Compairer::CompairResult result = compairer->compair(heroInts, villain.get_hands(), board);
            totalWeight += 1.0;
            if(result == Compairer::CompairResult::LARGER) wins += 1.0;
            else if(result == Compairer::CompairResult::EQUAL) ties += 1.0;
        }
    }

    if(totalWeight <= 0.0) return 0.5f;
    return (float)((wins + ties * 0.5) / totalWeight);
}
