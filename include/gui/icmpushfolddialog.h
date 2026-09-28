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
    void onPlayerCountChanged(int count);
    void onStacksContinue();
    void onPayoutCountChanged(int count);
    void onPayoutsContinue();
    void onPickHandClicked();
    void onHandPickerClosed();
    void onBack();

private:
    void rebuildStackRows(int count);
    void rebuildPayoutRows(int count);
    void showResult();

    std::shared_ptr<Compairer> compairer;
    QStackedWidget* stack;

    // Page 0: scenario
    PushFoldScenario scenario = PushFoldScenario::OpenShove;

    // Page 1: players/stacks
    QVBoxLayout* stackRowsLayout;
    QVector<QLineEdit*> stackInputs;
    QVector<QComboBox*> roleInputs; // "", "Vos", "Rival" per row

    // Page 2: blinds/payouts
    QLineEdit* bigBlindInput;
    QVBoxLayout* payoutRowsLayout;
    QVector<QLineEdit*> payoutInputs;

    // Page 3: hand
    QTextEdit* handTextEdit; // reused as the boardselector's target, holds "Jh,Jd"
    QLabel* handLabel;

    // Page 4: result
    QLabel* resultLabel;
    QLabel* handCard1Chip;
    QLabel* handCard2Chip;
    QLabel* evCallBox;
    QLabel* evFoldBox;
};

#endif // ICMPUSHFOLDDIALOG_H
