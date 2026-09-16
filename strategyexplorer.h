#ifndef STRATEGYEXPLORER_H
#define STRATEGYEXPLORER_H

#include <QDialog>
#include <QTimer>
#include <tuple>
#include <QMouseEvent>
#include <QEvent>
#include <QMouseEvent>

#include "include/runtime/qsolverjob.h"
#include "QItemSelection"
#include "include/ui/worditemdelegate.h"
#include "include/ui/tablestrategymodel.h"
#include "include/ui/strategyitemdelegate.h"
#include "include/ui/detailwindowsetting.h"
#include "include/Card.h"
#include "include/ui/detailviewermodel.h"
#include "include/ui/detailitemdelegate.h"
#include "include/ui/roughstrategyviewermodel.h"
#include "include/ui/roughstrategyitemdelegate.h"
#include "include/nodes/GameTreeNode.h"
#include "include/nodes/ActionNode.h"
#include "include/nodes/ChanceNode.h"
#include "include/nodes/TerminalNode.h"
#include "include/nodes/ShowdownNode.h"

namespace Ui {
class StrategyExplorer;
}

class StrategyExplorer : public QDialog
{
    Q_OBJECT

public:
    explicit StrategyExplorer(QWidget *parent = 0,QSolverJob * qSolverJob=nullptr);
    ~StrategyExplorer();
    void selectRootNode();
    void setHighlightedHand(QString card1, QString card2);
    // Returns (foldPct, callPct, betPct) for the currently selected node's
    // full aggregate strategy, 0..1 each. Used by the practice quiz to
    // compare the user's guess against the solver's recommendation.
    std::tuple<float,float,float> getRootActionSummary();
    // Stops the 1s auto-refresh timer. Must be called before a re-solve
    // starts rebuilding the same QSolverJob's tree in place — otherwise the
    // timer keeps polling into the tree while it's being torn down/rebuilt
    // on the background thread and crashes.
    void stopAutoUpdate();
    // Hides the manual turn/river runout picker (only meaningful when the
    // whole tree, including every runout, was solved at once — which is
    // what Quick Mode does. There, the picker duplicates the dedicated
    // "choose the turn/river and re-solve" bar on the results screen).
    void setRunoutPickerVisible(bool visible);

private:
    void setAdvancedViewVisible(bool visible);
    bool advancedViewVisible = false;
    DetailWindowSetting detailWindowSetting;
    QTimer *timer;
    Ui::StrategyExplorer *ui;
    QSolverJob * qSolverJob;
    StrategyItemDelegate * delegate_strategy;
    TableStrategyModel * tableStrategyModel;
    DetailViewerModel * detailViewerModel;
    DetailItemDelegate * detailItemItemDelegate;
    RoughStrategyViewerModel * roughStrategyViewerModel;
    RoughStrategyItemDelegate * roughStrategyItemDelegate;
    vector<Card> cards;
    bool advancedMode = false;
    void process_treeclick(TreeItem* treeitem);
    void process_board(TreeItem* treeitem);
    void updateRangeSummaryLabel();
public slots:
    void item_expanded(const QModelIndex& index);
    void item_clicked(const QModelIndex& index);
    void selection_changed(const QItemSelection &selected,
                                            const QItemSelection &deselected);
private slots:
    void on_turnCardBox_currentIndexChanged(int index);
    void on_riverCardBox_currentIndexChanged(int index);
    void update_second();
    void onMouseMoveEvent(int i,int j);
    void on_strategyModeButtom_clicked();
    void on_ipRangeButtom_clicked();
    void on_oopRangeButtom_clicked();
    void on_evModeButtom_clicked();
    void on_evOnlyModeButtom_clicked();
    void on_advancedModeCheck_toggled(bool checked);
    void on_toggleAdvancedViewButton_clicked();
    void on_exportImageButton_clicked();
    void on_glossaryButton_clicked();
};

#endif // STRATEGYEXPLORER_H
