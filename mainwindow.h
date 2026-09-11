#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPlainTextEdit>
#include "include/runtime/qsolverjob.h"
#include "qstextedit.h"
#include "strategyexplorer.h"
#include "rangeselector.h"
#include <QMessageBox>
#include "boardselector.h"
#include "settingeditor.h"
#include "welcomedialog.h"
#include "include/ui/rangeselectortablemodel.h"
#include "include/ui/rangeselectortabledelegate.h"
#include "include/data/quickmoderanges.h"
#include "include/ui/boardselectortablemodel.h"
#include "include/ui/boardselectortabledelegate.h"
#include <QProgressDialog>
#include <QPushButton>
#include <QVector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static QSTextEdit * s_textEdit;
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    Ui::MainWindow * getPriUi();
    QSTextEdit * getLogArea();
    QSTextEdit * get_logwindow();

private slots:
    void on_buttomSolve_clicked();
    void on_clearLogButtom_clicked();
    void on_buildTreeButtom_clicked();
    void on_actionjson_triggered();
    void on_actionimport_triggered();
    void on_wizardLoadConfigButton_clicked();
    void import_from_file(QString from);
    void on_actionexport_triggered();
    void on_actionSettings_triggered();
    void on_actionclear_all_triggered();
    void on_ip_range(QString range_text);
    void on_copyButtom_clicked();
    void on_showResultButton_clicked();
    void on_stopSolvingButton_clicked();
    void on_ipRangeSelectButtom_clicked();
    void on_oopRangeSelectButtom_clicked();
    void on_estimateMemoryButtom_clicked();
    void on_selectBoardButton_clicked();
    void on_actionopen_parameters_folder_triggered();
    void on_actionview_log_triggered();
    void onSolverJobFinished();

    void on_ipRangeText_textChanged();

    void on_oopRangeText_textChanged();

    void on_wizardBackButton_clicked();
    void on_wizardNextButton_clicked();
    void onIpRangeHover(int i, int j);
    void onOopRangeHover(int i, int j);
    void on_helpButton_clicked();
    void onHandSelectorClicked(const QModelIndex &index);
    void onSeatClicked();
    void onTableSize6Clicked();
    void onTableSize9Clicked();
    void onNewHandButtonClicked();

private:
    void clear_all_params();
    void showWizardStep(int index);
    void showWelcomeIfNeeded();
    void resetToExampleDefaults();
    bool exampleMode = false;
    bool solvingInProgress = false;
    enum class QuickModeStage { None, WaitingForBuildTree, WaitingForSolve };
    QuickModeStage quickModePendingStage = QuickModeStage::None;
    bool quickMode = false;
    bool quizMode = false;
    QString quizGuessedAction; // "FOLD", "CALL" or "BET"
    int currentQuickStep = 0;
    QWidget* quickModeSteps[4];
    QuickModeMatchup currentMatchup;
    bool userIsOpener = true;
    QString quickModeCard1;
    QString quickModeCard2;
    QString mySeat;
    QString villainSeat;
    int quickTableSize = 6; // 6 (6-max) or 9 (Full Ring)
    QVector<QPushButton*> quickSeatButtons; // pool of up to 9, repositioned per table size
    QStringList quickSeatDisplayOrder; // seat codes currently shown, clockwise from top
    BoardSelectorTableModel* handSelectorModel = NULL;
    BoardSelectorTableDelegate* handSelectorDelegate = NULL;
    QProgressDialog* quickModeProgressDialog = NULL;
    void startQuickMode();
    void startPracticeQuiz();
    void showQuickModeStep(int index);
    void startQuickModeSolve(bool fastMode = false);
    void resetSeatSelection();
    void updateSeatButtonStyles();
    void setupQuickSeatButtons(int tableSize);
    void onTableSizeChosen(int tableSize);
    Ui::MainWindow *ui = NULL;
    QSolverJob* qSolverJob = NULL;
    QDialog* logDialog = NULL;
    StrategyExplorer* strategyExplorer = NULL;
    RangeSelector* rangeSelector = NULL;
    boardselector* boardSelector = NULL;
    SettingEditor* settingEditor = NULL;
    int currentWizardStep = 0;
    QWidget* wizardSteps[6];
    RangeSelectorTableDelegate * ip_delegate;
    RangeSelectorTableDelegate * oop_delegate;
    RangeSelectorTableModel * ip_model;
    RangeSelectorTableModel * oop_model;

};

#endif // MAINWINDOW_H
