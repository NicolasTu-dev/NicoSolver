#ifndef ICMPUSHFOLDDIALOG_H
#define ICMPUSHFOLDDIALOG_H

#include <QDialog>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QTextEdit>
#include <QVector>
#include <memory>
#include <functional>
#include "include/data/pushfoldranges.h"
#include "include/compairer/Compairer.h"

// Self-contained flow for the ICM push/fold helper: table/stacks ->
// blinds/payouts -> hero's hand -> recommendation. Opened as a modal
// dialog from a button inside Modo Rapido's situation-picker step;
// independent of the postflop CFR flow (no GameTree, no PokerSolver
// solving happens here).
class IcmPushFoldDialog : public QDialog {
    Q_OBJECT
public:
    explicit IcmPushFoldDialog(std::shared_ptr<Compairer> compairer, QWidget* parent = nullptr);

private slots:
    void onScenarioContinue();
    void onHeroVillainContinue();
    void onOtherPlayersContinue();
    void onBigBlindContinue();
    void onPayoutsContinue();
    void onPickHandClicked();
    void onHandPickerClosed();
    void onBack();

private:
    void rebuildOtherPlayerRows();
    void rebuildPayoutRows();
    void showResult();
    void goToPage(int index);
    QWidget* buildProgressDots(int currentStep);
    QWidget* buildStepperRow(int* value, int minValue, int maxValue, QLabel** outValueLabel, std::function<void()> onChange);

    std::shared_ptr<Compairer> compairer;
    QStackedWidget* stack;
    QVector<int> navHistory;

    // Page 0: scenario
    PushFoldScenario scenario = PushFoldScenario::OpenShove;

    // Page 1: how many players
    int playerCount = 6;
    QLabel* playerCountLabel;

    // Page 2: your stack and your opponent's
    QLineEdit* heroStackInput;
    QLineEdit* villainStackInput;

    // Page 3: the rest of the table (playerCount - 2 rows; skipped if playerCount == 2)
    QVBoxLayout* otherPlayersLayout;
    QVector<QLineEdit*> otherPlayerInputs;

    // Page 4: big blind
    QLineEdit* bigBlindInput;

    // Page 5: payouts
    int payoutCount = 3;
    QLabel* payoutCountLabel;
    QVBoxLayout* payoutRowsLayout;
    QVector<QLineEdit*> payoutInputs;

    // Page 6: hand
    QTextEdit* handTextEdit; // reused as the boardselector's target, holds "Jh,Jd"
    QLabel* handLabel;

    // Page 7: result
    QLabel* resultLabel;
    QLabel* handCard1Chip;
    QLabel* handCard2Chip;
    QLabel* evCallBox;
    QLabel* evFoldBox;
};

#endif // ICMPUSHFOLDDIALOG_H
