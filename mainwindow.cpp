#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "stdio.h"
#include "include/runtime/qsolverjob.h"
#include <QFileDialog>
#include <QSettings>
#include "include/library.h"
#include <QToolTip>
#include <QCursor>
#include <QVBoxLayout>
#include <QMap>
#include <QGraphicsDropShadowEffect>
#include <QRandomGenerator>
#include <tuple>
#include "include/Card.h"
#include <QFrame>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

QSTextEdit* MainWindow::s_textEdit = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    MainWindow::s_textEdit = this->get_logwindow();
    connect(this->ui->actionSettings, &QAction::triggered, this, &MainWindow::on_actionSettings_triggered);
    connect(this->ui->actionimport, &QAction::triggered, this, &MainWindow::on_actionimport_triggered);
    connect(this->ui->actionexport, &QAction::triggered, this, &MainWindow::on_actionexport_triggered);
    connect(this->ui->actionclear_all, &QAction::triggered, this, &MainWindow::on_actionclear_all_triggered);
    connect(this->ui->actionopen_parameters_folder, &QAction::triggered, this, &MainWindow::on_actionopen_parameters_folder_triggered);
    connect(this->ui->actionview_log, &QAction::triggered, this, &MainWindow::on_actionview_log_triggered);
    qSolverJob = new QSolverJob;
    qSolverJob->setContext(this->getLogArea());
    qSolverJob->current_mission = QSolverJob::MissionType::LOADING;
    qSolverJob->start();
    connect(qSolverJob, &QThread::finished, this, &MainWindow::onSolverJobFinished);
    this->setWindowTitle(tr("Solverix"));

    // Re-check the subscription against the server periodically. Checking
    // only the locally cached expiry date (set at login) can't detect an
    // early cancellation made from another device — this closes that gap
    // without needing to hit the network on every single action.
    this->subscriptionCheckTimer = new QTimer(this);
    this->subscriptionCheckTimer->setInterval(60 * 60 * 1000); // 1 hour
    connect(this->subscriptionCheckTimer, &QTimer::timeout, this, &MainWindow::onSubscriptionCheckTimer);
    this->subscriptionCheckTimer->start();

    // The log view isn't part of the main wizard anymore; it lives in its own
    // dialog reachable from Solver -> "Ver registro (log)" so the wizard gets
    // the full window width.
    this->logDialog = new QDialog(this);
    this->logDialog->setWindowTitle(tr("Log"));
    this->logDialog->resize(700, 500);
    QVBoxLayout* logDialogLayout = new QVBoxLayout(this->logDialog);
    logDialogLayout->addWidget(this->ui->logOutput);
    logDialogLayout->addWidget(this->ui->clearLogButtom);

    // Thumbnail process
    this->ip_model = new RangeSelectorTableModel(QString("A,K,Q,J,T,9,8,7,6,5,4,3,2").split(","),this->ui->ipRangeText->toPlainText(),this,true);
    this->ip_delegate = new RangeSelectorTableDelegate(QString("A,K,Q,J,T,9,8,7,6,5,4,3,2").split(","),this->ip_model,this);
    this->ui->IpRangeTableView->setModel(this->ip_model);
    this->ui->IpRangeTableView->setItemDelegate(this->ip_delegate);
    this->ui->IpRangeTableView->verticalHeader()->setMinimumSectionSize(1);
    this->ui->IpRangeTableView->horizontalHeader()->setMinimumSectionSize(1);

    this->oop_model = new RangeSelectorTableModel(QString("A,K,Q,J,T,9,8,7,6,5,4,3,2").split(","),this->ui->oopRangeText->toPlainText(),this,true);
    this->oop_delegate = new RangeSelectorTableDelegate(QString("A,K,Q,J,T,9,8,7,6,5,4,3,2").split(","),this->oop_model,this);
    this->ui->oopRangeTableView->setModel(this->oop_model);
    this->ui->oopRangeTableView->setItemDelegate(this->oop_delegate);
    this->ui->oopRangeTableView->verticalHeader()->setMinimumSectionSize(1);
    this->ui->oopRangeTableView->horizontalHeader()->setMinimumSectionSize(1);
    this->ui->tabWidget->hide();

    connect(this->ui->IpRangeTableView, SIGNAL(itemMouseChange(int,int)), this, SLOT(onIpRangeHover(int,int)));
    connect(this->ui->oopRangeTableView, SIGNAL(itemMouseChange(int,int)), this, SLOT(onOopRangeHover(int,int)));

    // Quick-mode hand picker — reuses the existing board-card grid widget/model,
    // just pointed at a separate (initially empty) text buffer for "my 2 cards".
    this->handSelectorModel = new BoardSelectorTableModel(QString("A,K,Q,J,T,9,8,7,6,5,4,3,2").split(","), "", this);
    this->ui->handSelectorTable->setModel(this->handSelectorModel);
    this->handSelectorDelegate = new BoardSelectorTableDelegate(QString("A,K,Q,J,T,9,8,7,6,5,4,3,2").split(","), this->handSelectorModel, this);
    this->ui->handSelectorTable->setItemDelegate(this->handSelectorDelegate);
    connect(this->ui->handSelectorTable, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onHandSelectorClicked(const QModelIndex&)));

    this->quickModeSteps[0] = this->ui->quickStepSituation;
    this->quickModeSteps[1] = this->ui->quickStepHand;
    this->quickModeSteps[2] = this->ui->wizardStepBoard;
    this->quickModeSteps[3] = this->ui->quickStepResults;
    connect(this->ui->newHandButton, &QPushButton::clicked, this, &MainWindow::onNewHandButtonClicked);
    this->ui->quickStepSituation->setVisible(false);
    this->ui->quickStepHand->setVisible(false);
    this->ui->quickStepResults->setVisible(false);

    connect(this->ui->tableSize6Button, &QPushButton::clicked, this, &MainWindow::onTableSize6Clicked);
    connect(this->ui->tableSize9Button, &QPushButton::clicked, this, &MainWindow::onTableSize9Clicked);
    this->setupQuickSeatButtons(6);

    // Soft depth on the poker table felt, so it reads as a lifted surface
    // rather than a flat green rectangle.
    auto feltShadow = new QGraphicsDropShadowEffect(this);
    feltShadow->setBlurRadius(30);
    feltShadow->setOffset(0, 4);
    feltShadow->setColor(QColor(0, 0, 0, 160));
    this->ui->seatFelt->setGraphicsEffect(feltShadow);

    this->ui->wizardLoadConfigButton->setToolTip(tr("If you already saved a configuration before (a .json file), tap here to load it and skip every step."));
    this->ui->ipRangeText->setToolTip(tr("The IP player's (the one who acts last) hand range shows up here as text. You can type it by hand or build it with the grid on the right by tapping \"Select IP\"."));
    this->ui->oopRangeText->setToolTip(tr("The OOP player's (the one who acts first) hand range shows up here as text. You can type it by hand or build it with the grid on the right by tapping \"Select OOP\"."));
    this->ui->IpRangeTableView->setToolTip(tr("Grid of possible hands for the IP player. Each cell is a card combination: tap a cell to include it (green) or exclude it from the range. The darker the green, the more often that hand is played."));
    this->ui->oopRangeTableView->setToolTip(tr("Grid of possible hands for the OOP player. Each cell is a card combination: tap a cell to include it (green) or exclude it from the range. The darker the green, the more often that hand is played."));
    this->ui->ipRangeSelectButtom->setToolTip(tr("Opens the visual grid to build the IP player's range by tapping hands instead of typing text."));
    this->ui->oopRangeSelectButtom->setToolTip(tr("Opens the visual grid to build the OOP player's range by tapping hands instead of typing text."));

    QString allinTooltip = tr("When checked (✓ green), the solver adds the option to go all-in with the whole stack on this street, in addition to the bet sizes defined above.");
    this->ui->flop_ip_allin->setToolTip(allinTooltip);
    this->ui->turn_ip_allin->setToolTip(allinTooltip);
    this->ui->river_ip_allin->setToolTip(allinTooltip);
    this->ui->flop_oop_allin->setToolTip(allinTooltip);
    this->ui->turn_oop_allin->setToolTip(allinTooltip);
    this->ui->river_oop_allin->setToolTip(allinTooltip);

    this->ui->groupBox->setToolTip(tr("Flop = the first 3 community cards. Here you set which bet sizes IP (the player who acts last) can use when it's their turn to open the action on the flop. Example: entering \"50\" in Bet Sizes = betting 50% of the pot at that moment."));
    this->ui->groupBox_2->setToolTip(tr("Turn = the 4th community card. Bet sizes IP can use when opening the action on the turn. Same idea as the flop: the numbers are % of the pot, not fixed chips."));
    this->ui->groupBox_3->setToolTip(tr("River = the 5th and last community card. Bet sizes IP can use when opening the action on the river."));
    this->ui->groupBox_4->setToolTip(tr("Flop = the first 3 community cards. Bet sizes OOP (the player who acts first) can use when deciding to open betting on the flop, instead of checking."));
    this->ui->groupBox_5->setToolTip(tr("Turn = the 4th community card. \"Donk Sizes\" is for this specific case: nobody bet on the flop (both checked), the turn arrives, and OOP decides to bet first instead of waiting to see what IP does — that's called a \"donk bet\". The numbers are % of the pot at that moment."));
    this->ui->groupBox_6->setToolTip(tr("River = the 5th and last community card. Same case as the turn: \"Donk Sizes\" are the sizes OOP can use to bet first on the river after not betting on the turn."));

    this->ui->raiseLimitText->setToolTip(tr("Maximum number of consecutive raises the solver considers on the same street. A higher number makes the tree much bigger and slower to solve."));
    this->ui->potText->setToolTip(tr("Pot size before this situation, in chips."));
    this->ui->effectiveStackText->setToolTip(tr("Chips left for the player with the smaller stack. That's the most that can be bet in the hand."));
    this->ui->mode_box->setToolTip(tr("Deck to use: \"texas holdem\" (52 normal cards) or \"shortdeck\" (36 cards, no 2, 3, 4, or 5)."));
    this->ui->allinThresholdText->setToolTip(tr("If a player's remaining stack is less than this % of the pot, the solver directly offers going all-in instead of intermediate bet sizes."));
    this->ui->useIsoCheck->setToolTip(tr("Internal optimization that groups equivalent cards together to solve faster without losing precision. Recommended to leave checked."));
    this->ui->useHalfFloats_box->setToolTip(tr("Reduces the RAM used by the solver at the cost of solving slower or with less numerical precision. Only use it if you're running out of memory."));
    this->ui->iterationText->setToolTip(tr("Maximum number of times the solver recalculates the strategy. More iterations = more precision, but takes longer."));
    this->ui->exploitabilityText->setToolTip(tr("The solver stops early if it already reached a strategy with this error level (% of the pot) or less. A lower number is more precise but slower; 0.5% is already a very solid strategy."));
    this->ui->logIntervalText->setToolTip(tr("How often progress is shown in the console below, in iterations. Only affects how often you see updates, not the final result."));
    this->ui->threadsText->setToolTip(tr("Number of processor cores the solver can use at once. More threads = solves faster if your computer has free cores."));

    this->wizardSteps[0] = this->ui->wizardStepRanges;
    this->wizardSteps[1] = this->ui->wizardStepBoard;
    this->wizardSteps[2] = this->ui->wizardStepBetSizes;
    this->wizardSteps[3] = this->ui->wizardStepTreeParams;
    this->wizardSteps[4] = this->ui->wizardStepSolverOptions;
    this->wizardSteps[5] = this->ui->wizardStepConfirm;
    this->showWizardStep(0);

    this->showWelcomeIfNeeded();
}

void MainWindow::showWelcomeIfNeeded()
{
    QSettings setting("Solverix", "Setting");
    setting.beginGroup("solver");
    bool seen_welcome = setting.value("seen_welcome", false).toBool();

    if(!seen_welcome){
        setting.setValue("seen_welcome", true);
        this->on_helpButton_clicked();
    }
}

void MainWindow::resetToExampleDefaults()
{
    // A BTN-open-vs-BB-call spot on a Q-J-2 flop; these are the same
    // values the app already ships as its default field contents.
    this->ui->ipRangeText->setPlainText(
        "AA,KK,QQ,JJ,TT,99:0.75,88:0.75,77:0.5,66:0.25,55:0.25,AK,AQs,AQo:0.75,AJs,AJo:0.5,ATs:0.75,A6s:0.25,A5s:0.75,A4s:0.75,A3s:0.5,A2s:0.5,KQs,KQo:0.5,KJs,KTs:0.75,K5s:0.25,K4s:0.25,QJs:0.75,QTs:0.75,Q9s:0.5,JTs:0.75,J9s:0.75,J8s:0.75,T9s:0.75,T8s:0.75,T7s:0.75,98s:0.75,97s:0.75,96s:0.5,87s:0.75,86s:0.5,85s:0.5,76s:0.75,75s:0.5,65s:0.75,64s:0.5,54s:0.75,53s:0.5,43s:0.5");
    this->ui->oopRangeText->setPlainText(
        "QQ:0.5,JJ:0.75,TT,99,88,77,66,55,44,33,22,AKo:0.25,AQs,AQo:0.75,AJs,AJo:0.75,ATs,ATo:0.75,A9s,A8s,A7s,A6s,A5s,A4s,A3s,A2s,KQ,KJ,KTs,KTo:0.5,K9s,K8s,K7s,K6s,K5s,K4s:0.5,K3s:0.5,K2s:0.5,QJ,QTs,Q9s,Q8s,Q7s,JTs,JTo:0.5,J9s,J8s,T9s,T8s,T7s,98s,97s,96s,87s,86s,76s,75s,65s,64s,54s,53s,43s");
    this->ui->boardText->setPlainText("Qs,Jh,2h");
    this->ui->potText->setText("50");
    this->ui->effectiveStackText->setText("200");
    this->exampleMode = true;
    this->showWizardStep(0);
}

bool MainWindow::requirePlan(LicenseManager::Plan minPlan){
    LicenseManager::Plan current = LicenseManager::currentPlan();
    bool ok = (minPlan == LicenseManager::Plan::Advanced)
        ? (current == LicenseManager::Plan::Advanced || current == LicenseManager::Plan::Complete)
        : (current == LicenseManager::Plan::Complete);
    if(ok) return true;

    QString needed = LicenseManager::planDisplayName(minPlan);
    QMessageBox::information(this, tr("You need a subscription"),
        current == LicenseManager::Plan::None
            ? tr("You haven't activated any subscription yet. You need the \"%1\" plan or higher for this.").arg(needed)
            : tr("Your current plan doesn't include this. You need the \"%1\" plan or higher.").arg(needed));
    LicenseDialog licenseDialog(this);
    licenseDialog.exec();
    LicenseManager::Plan afterDialog = LicenseManager::currentPlan();
    return (minPlan == LicenseManager::Plan::Advanced)
        ? (afterDialog == LicenseManager::Plan::Advanced || afterDialog == LicenseManager::Plan::Complete)
        : (afterDialog == LicenseManager::Plan::Complete);
}

void MainWindow::on_helpButton_clicked()
{
    WelcomeDialog dialog(this);
    dialog.exec();
    if(dialog.choice() == WelcomeDialog::QuickMode){
        if(!this->requirePlan(LicenseManager::Plan::Complete)) return;
        this->startQuickMode();
    }else if(dialog.choice() == WelcomeDialog::Advanced){
        if(!this->requirePlan(LicenseManager::Plan::Advanced)) return;
        this->quickMode = false;
        this->exampleMode = false;
        for(int i = 0; i < 3; i++){
            this->quickModeSteps[i]->setVisible(false);
        }
        this->showWizardStep(this->currentWizardStep);
    }else if(dialog.choice() == WelcomeDialog::Practice){
        if(!this->requirePlan(LicenseManager::Plan::Complete)) return;
        this->startPracticeQuiz();
    }
}

static QString quizCardChipsHtml(const QStringList& cardStrs){
    QString html;
    for(const QString& c : cardStrs){
        Card card(c.toStdString());
        html += QString("<span style='background-color:#132118;border:1px solid #2a4a38;"
                         "border-radius:8px;padding:8px 16px;margin-right:6px;"
                         "font-size:26px;font-weight:700;'>%1</span>")
            .arg(card.toFormattedHtml());
    }
    return html;
}

void MainWindow::startPracticeQuiz(){
    int tableSize = QRandomGenerator::global()->bounded(2) == 0 ? 6 : 9;
    QStringList seatOrder = getTableSeatOrder(tableSize);
    int seatAIdx = (int)QRandomGenerator::global()->bounded(seatOrder.size());
    int seatBIdx;
    do { seatBIdx = (int)QRandomGenerator::global()->bounded(seatOrder.size()); } while(seatBIdx == seatAIdx);
    QString openerSeat = seatAIdx < seatBIdx ? seatOrder[seatAIdx] : seatOrder[seatBIdx];
    QString callerSeat = seatAIdx < seatBIdx ? seatOrder[seatBIdx] : seatOrder[seatAIdx];
    QuickModeMatchup matchup = buildQuickModeMatchup(tableSize, openerSeat, callerSeat);
    bool iAmOpener = QRandomGenerator::global()->bounded(2) == 0;

    QStringList ranksList = {"A","K","Q","J","T","9","8","7","6","5","4","3","2"};
    QStringList suitsList = {"s","h","d","c"};
    QStringList usedCards;
    auto randomCard = [&]() -> QString {
        QString c;
        do {
            c = ranksList[QRandomGenerator::global()->bounded(ranksList.size())] +
                suitsList[QRandomGenerator::global()->bounded(suitsList.size())];
        } while(usedCards.contains(c));
        usedCards << c;
        return c;
    };
    QString hand1 = randomCard();
    QString hand2 = randomCard();
    QString flop1 = randomCard();
    QString flop2 = randomCard();
    QString flop3 = randomCard();

    QString situation = iAmOpener ? matchup.descriptionAsOpener : matchup.descriptionAsCaller;

    QDialog guessDialog(this);
    guessDialog.setWindowTitle(tr("Practice Mode"));
    guessDialog.setMinimumSize(600, 460);
    QVBoxLayout* layout = new QVBoxLayout(&guessDialog);
    layout->setSpacing(16);
    layout->setContentsMargins(28, 28, 28, 28);

    QLabel* titleLabel = new QLabel(tr("🎯 What would you do here?"), &guessDialog);
    titleLabel->setStyleSheet("font-size:22px; font-weight:800;");
    layout->addWidget(titleLabel);

    QLabel* situationLabel = new QLabel(situation, &guessDialog);
    situationLabel->setWordWrap(true);
    situationLabel->setStyleSheet("font-size:14px;");
    layout->addWidget(situationLabel);

    QFrame* divider = new QFrame(&guessDialog);
    divider->setFrameShape(QFrame::HLine);
    layout->addWidget(divider);

    QLabel* handTitle = new QLabel(tr("Your hand:"), &guessDialog);
    handTitle->setStyleSheet("font-size:13px; color:#a9c9b6; font-weight:700;");
    layout->addWidget(handTitle);
    QLabel* handLabel = new QLabel(quizCardChipsHtml({hand1, hand2}), &guessDialog);
    layout->addWidget(handLabel);

    QLabel* boardTitle = new QLabel(tr("Board:"), &guessDialog);
    boardTitle->setStyleSheet("font-size:13px; color:#a9c9b6; font-weight:700;");
    layout->addWidget(boardTitle);
    QLabel* boardLabel = new QLabel(quizCardChipsHtml({flop1, flop2, flop3}), &guessDialog);
    layout->addWidget(boardLabel);

    layout->addStretch();

    QLabel* questionLabel = new QLabel(tr("What would you do with this hand?"), &guessDialog);
    questionLabel->setStyleSheet("font-size:15px; font-weight:700;");
    layout->addWidget(questionLabel);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(12);
    QPushButton* foldBtn = new QPushButton(tr("Fold"), &guessDialog);
    QPushButton* callBtn = new QPushButton(tr("Call / Check"), &guessDialog);
    QPushButton* betBtn = new QPushButton(tr("Bet / Raise"), &guessDialog);
    QString bigButtonStyle = "font-size:15px; font-weight:700; padding:14px 10px;";
    foldBtn->setStyleSheet(bigButtonStyle);
    callBtn->setStyleSheet(bigButtonStyle);
    betBtn->setStyleSheet(bigButtonStyle);
    buttonsLayout->addWidget(foldBtn);
    buttonsLayout->addWidget(callBtn);
    buttonsLayout->addWidget(betBtn);
    layout->addLayout(buttonsLayout);

    QString guess;
    connect(foldBtn, &QPushButton::clicked, &guessDialog, [&](){ guess = "FOLD"; guessDialog.accept(); });
    connect(callBtn, &QPushButton::clicked, &guessDialog, [&](){ guess = "CALL"; guessDialog.accept(); });
    connect(betBtn, &QPushButton::clicked, &guessDialog, [&](){ guess = "BET"; guessDialog.accept(); });

    if(guessDialog.exec() != QDialog::Accepted || guess.isEmpty()) return;

    this->quizMode = true;
    this->quizGuessedAction = guess;
    this->currentMatchup = matchup;
    this->userIsOpener = iAmOpener;
    this->quickModeCard1 = hand1;
    this->quickModeCard2 = hand2;
    this->ui->boardText->setPlainText(QString("%1,%2,%3").arg(flop1, flop2, flop3));

    this->startQuickModeSolve(true);
}

void MainWindow::showWizardStep(int index)
{
    if(index < 0 || index > 5) return;
    this->currentWizardStep = index;
    for(int i = 0; i < 6; i++){
        this->wizardSteps[i]->setVisible(i == index);
    }
    QStringList stepNames;
    stepNames << tr("Ranges") << tr("Board") << tr("Bet Sizings")
              << tr("Tree Parameters") << tr("Solver Options") << tr("Confirm and solve");
    QStringList stepSubtitles;
    stepSubtitles
        << tr("A \"range\" is the set of hands a player might have in this situation. IP (\"in position\") is the player who acts last on the street; OOP (\"out of position\") is the one who acts first. Tap a grid cell to add or remove that hand from the range (darker = played more often), or type the range by hand in the text box.")
        << tr("Choose the cards that already came out on the table (3 for flop, 4 for turn, 5 for river).")
        << tr("This step defines WHAT BET SIZES the solver can choose — you don't need to understand every term now, the default values are already enough to try it out. Concrete example: if the pot has 100 chips and you put \"50\" in Bet Sizes, that means \"bet 50% of the pot\" = 50 chips. If someone then raises with \"60\" in Raise Sizes, that's 60% of the pot AFTER it grew from that bet (not the original pot) — that's why it's expressed in % instead of fixed chips, so it works for any pot size. There are 6 boxes because the game splits into 3 streets (Flop, Turn, River) and on each one IP and OOP can use different sizes; check the ▸ titles above each row to orient yourself. \"Donk Sizes\" is only for OOP on Turn/River: those are the sizes it can use to bet first on that new street, even though it was the one who didn't bet on the previous street (that play is called a \"donk bet\"). \"Add Allin\" simply adds, in addition to those sizes, the option to go straight all-in with the whole stack.")
        << tr("\"Raise limit\" is the maximum number of consecutive raises the solver will consider on the same street (more raises = bigger, slower tree). \"Pot\" is the pot size before this situation starts, and \"Effective Stack\" is the number of chips left for the player with the smaller stack (the most that can be bet). \"Mode\" sets the deck: \"texas holdem\" (52 cards) or \"shortdeck\" (36 cards, no 2-5). \"Allin threshold\" is a shortcut: if a player has less than that % of the pot left, the solver directly offers going all-in instead of intermediate bet sizes, to avoid unnecessarily complicating the tree. \"Use isomorphism\" is an internal optimization: it groups cards that are strategically equivalent (e.g. two suits that don't make a flush anywhere) to solve faster without losing precision — leave it checked unless you have a specific reason to disable it. \"Save memory at cost of speed/accuracy\" reduces the RAM used in exchange for solving a bit slower or with less numerical precision; only use it if you're running out of memory. Tapping \"Next\" builds the decision tree automatically with these values.")
        << tr("\"Iterations\" is the maximum number of times the solver will recalculate the strategy (more iterations = more precision, but more time). \"Stop solving when reach X% exploitability\" makes the solver stop early if it already reached a strategy close enough to optimal (a lower number = more precise but slower; 0.5% is already a very solid strategy to play). \"Log interval\" is how often progress is printed in the console below, it only affects how often you see updates, not the result. \"Threads\" is the number of processor cores the solver can use at once (more threads = solves faster if your computer has enough free cores). The default values work fine to start.")
        << tr("Review everything and tap \"Start solving\" so the solver calculates the optimal strategy.");
    this->ui->wizardStepLabel->setText(tr("Step %1 of 6: %2").arg(index + 1).arg(stepNames[index]));
    this->ui->wizardStepSubtitle->setText(stepSubtitles[index]);
    this->ui->wizardBackButton->setEnabled(index > 0);
    this->ui->wizardNextButton->setText(index == 5 ? tr("Done") : tr("Next →"));

    if(this->exampleMode){
        QStringList exampleTexts;
        exampleTexts
            << tr("📌 Example: IP is a player on the button (BTN) who opened the hand with a typical opening range. OOP is the big blind (BB) who called that open with their defending range. Notice both ranges are already loaded in the grid.")
            << tr("📌 Example: the flop came Q♠ J♥ 2♥ — a high card (Q), a middle card that could have connected with several hands (J), and a low card that also brings a flush draw in hearts.")
            << tr("📌 Example: we left the bet sizes at default (50% of the pot when betting, 60% when raising) to keep the example simple. In a real hand you'd adjust this to how your tables usually play.")
            << tr("📌 Example: the pot before this situation is 50 chips and each player has 200 chips of effective stack left — round numbers chosen to make it easy to follow.")
            << tr("📌 Example: we left 200 iterations, which is enough for the solver to converge quickly on a simple example like this (on a more complex hand you might need more).")
            << tr("📌 Example: tap \"Start solving\" and wait for it to finish (with these values it should take just seconds). Then tap \"ShowResult\" to see the recommended strategy.");
        this->ui->exampleBanner->setText(exampleTexts[index]);
        this->ui->exampleBanner->setVisible(true);
    }else{
        this->ui->exampleBanner->setVisible(false);
    }
}

static QString rangeCellTooltip(int i, int j, float freq){
    QStringList ranks = QString("A,K,Q,J,T,9,8,7,6,5,4,3,2").split(",");
    if(i < 0 || j < 0 || i >= ranks.size() || j >= ranks.size()) return QString();
    int larger = i > j ? i : j;
    int smaller = i > j ? j : i;
    QString hand = ranks[smaller] + ranks[larger];
    if(i > j) hand += "o";
    else if(i < j) hand += "s";
    int pct = (int)(freq * 100 + 0.5f);
    return QString("%1 — %2%").arg(hand).arg(pct);
}

void MainWindow::onIpRangeHover(int i, int j){
    QString text = rangeCellTooltip(i, j, this->ip_model->getRangeAt(i, j));
    if(!text.isEmpty()) QToolTip::showText(QCursor::pos(), text, this->ui->IpRangeTableView);
}

void MainWindow::onOopRangeHover(int i, int j){
    QString text = rangeCellTooltip(i, j, this->oop_model->getRangeAt(i, j));
    if(!text.isEmpty()) QToolTip::showText(QCursor::pos(), text, this->ui->oopRangeTableView);
}

void MainWindow::on_wizardBackButton_clicked()
{
    if(this->quickMode){
        if(this->currentQuickStep > 0){
            if(this->currentQuickStep - 1 == 0){
                this->resetSeatSelection();
            }
            this->showQuickModeStep(this->currentQuickStep - 1);
        }
        return;
    }
    if(this->currentWizardStep > 0){
        this->showWizardStep(this->currentWizardStep - 1);
    }
}

void MainWindow::on_wizardNextButton_clicked()
{
    if(this->quickMode){
        if(this->currentQuickStep == 0){
            if(this->mySeat.isEmpty() || this->villainSeat.isEmpty()){
                QMessageBox::information(this, tr("Choose both seats"), tr("Tap your seat and then your opponent's before continuing."));
                return;
            }
        }
        if(this->currentQuickStep == 1){
            QString handText = this->handSelectorModel->getBoardText();
            QStringList cards = handText.split(",", Qt::SkipEmptyParts);
            if(cards.size() != 2){
                QMessageBox::information(this, tr("Choose 2 cards"), tr("You need to tap exactly 2 cards for your hand before continuing."));
                return;
            }
            this->quickModeCard1 = cards[0];
            this->quickModeCard2 = cards[1];
        }
        if(this->currentQuickStep == 2){
            this->startQuickModeSolve();
            return;
        }
        if(this->currentQuickStep < 2){
            this->showQuickModeStep(this->currentQuickStep + 1);
        }
        return;
    }
    if(this->currentWizardStep == 3){
        // Leaving the tree-params step: build the tree automatically,
        // exactly what clicking the pre-existing "Build Tree" button already does.
        this->on_buildTreeButtom_clicked();
    }
    if(this->currentWizardStep < 5){
        this->showWizardStep(this->currentWizardStep + 1);
    }
}

QSTextEdit * MainWindow::get_logwindow(){
    return this->ui->logOutput;
}

MainWindow::~MainWindow()
{
    delete qSolverJob;
    delete ip_delegate;
    delete ip_model;
    delete oop_delegate;
    delete oop_model;
    delete ui;
}

QString getParams(QString input,QString key){
    if(input.contains(key)){
        return input.replace(key,"").trimmed();
    }else{
        return "INVALID";
    }
}

void MainWindow::on_actionclear_all_triggered(){
    this->clear_all_params();
    this->ui->IpRangeTableView->update();
    this->ui->oopRangeTableView->update();
    this->ui->IpRangeTableView->setFocus();
    this->ui->oopRangeTableView->setFocus();
}

void MainWindow::clear_all_params(){
    this->ui->potText->clear();
    this->ui->effectiveStackText->clear();
    this->ui->boardText->clear();
    this->ui->oopRangeText->clear();
    this->ui->ipRangeText->clear();
    this->ui->flop_oop_bet->clear();
    this->ui->flop_oop_raise->clear();
    this->ui->flop_oop_allin->setChecked(false);
    this->ui->flop_ip_bet->clear();
    this->ui->flop_ip_raise->clear();
    this->ui->flop_ip_allin->setChecked(false);
    this->ui->turn_oop_bet->clear();
    this->ui->turn_oop_raise->clear();
    this->ui->turn_oop_donk->clear();
    this->ui->turn_oop_allin->setChecked(false);
    this->ui->turn_ip_bet->clear();
    this->ui->turn_ip_raise->clear();
    this->ui->turn_ip_allin->setChecked(false);
    this->ui->river_oop_bet->clear();
    this->ui->river_oop_raise->clear();
    this->ui->river_oop_donk->clear();
    this->ui->river_oop_allin->setChecked(false);
    this->ui->river_ip_bet->clear();
    this->ui->river_ip_raise->clear();
    this->ui->river_ip_allin->setChecked(false);
    this->ui->allinThresholdText->clear();
    this->ui->threadsText->clear();
    this->ui->exploitabilityText->clear();
    this->ui->iterationText->clear();
    this->ui->logIntervalText->clear();
    this->ui->raiseLimitText->clear();
    this->ui->useIsoCheck->setChecked(false);
}

void MainWindow::import_from_file(QString fileName){
    if( fileName.isNull() )
    {
        qDebug().noquote() << tr("File selection invalid.");
        return;
    }
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug().noquote() << tr("File open failed.");
        return;
    }
    QString content;
    QTextStream s1(&file);
    content.append(s1.readAll());
    this->clear_all_params();
    for(QString one_line_content:content.split("\n")){
        if(getParams(one_line_content,"set_pot") != "INVALID"){
            this->ui->potText->setText(getParams(one_line_content,"set_pot"));
        }
        else if(getParams(one_line_content,"set_effective_stack") != "INVALID"){
            this->ui->effectiveStackText->setText(getParams(one_line_content,"set_effective_stack"));
        }
        else if(getParams(one_line_content,"set_board") != "INVALID"){
            this->ui->boardText->setText(getParams(one_line_content,"set_board"));
        }
        else if(getParams(one_line_content,"set_range_oop") != "INVALID"){
            this->ui->oopRangeText->setText(getParams(one_line_content,"set_range_oop"));
        }
        else if(getParams(one_line_content,"set_range_ip") != "INVALID"){
            this->ui->ipRangeText->setText(getParams(one_line_content,"set_range_ip"));
        }
        // FLOP
        else if(getParams(one_line_content,"set_bet_sizes oop,flop,bet,") != "INVALID"){
            this->ui->flop_oop_bet->setText(getParams(one_line_content,"set_bet_sizes oop,flop,bet,").replace(',',' '));
        }
        else if(getParams(one_line_content,"set_bet_sizes oop,flop,raise") != "INVALID"){
            this->ui->flop_oop_raise->setText(getParams(one_line_content,"set_bet_sizes oop,flop,raise").replace(',',' '));
        }
        else if(getParams(one_line_content,"set_bet_sizes oop,flop,allin") != "INVALID"){
            this->ui->flop_oop_allin->setChecked(true);
        }
        else if(getParams(one_line_content,"set_bet_sizes ip,flop,bet,") != "INVALID"){
            this->ui->flop_ip_bet->setText(getParams(one_line_content,"set_bet_sizes ip,flop,bet,").replace(',',' '));
        }
        else if(getParams(one_line_content,"set_bet_sizes ip,flop,raise") != "INVALID"){
            this->ui->flop_ip_raise->setText(getParams(one_line_content,"set_bet_sizes ip,flop,raise").replace(',',' '));
        }
        else if(getParams(one_line_content,"set_bet_sizes ip,flop,allin") != "INVALID"){
            this->ui->flop_ip_allin->setChecked(true);
        }
        // TURN
        else if(getParams(one_line_content,"set_bet_sizes oop,turn,bet,") != "INVALID"){
            this->ui->turn_oop_bet->setText(getParams(one_line_content,"set_bet_sizes oop,turn,bet,").replace(',',' '));
        }
        else if(getParams(one_line_content,"set_bet_sizes oop,turn,raise") != "INVALID"){
            this->ui->turn_oop_raise->setText(getParams(one_line_content,"set_bet_sizes oop,turn,raise").replace(',',' '));
        }
        else if(getParams(one_line_content,"set_bet_sizes oop,turn,donk") != "INVALID"){
            this->ui->turn_oop_donk->setText(getParams(one_line_content,"set_bet_sizes oop,turn,donk").replace(',',' '));
        }
        else if(getParams(one_line_content,"set_bet_sizes oop,turn,allin") != "INVALID"){
            this->ui->turn_oop_allin->setChecked(true);
        }
        else if(getParams(one_line_content,"set_bet_sizes ip,turn,bet,") != "INVALID"){
            this->ui->turn_ip_bet->setText(getParams(one_line_content,"set_bet_sizes ip,turn,bet,").replace(',',' '));
        }
        else if(getParams(one_line_content,"set_bet_sizes ip,turn,raise") != "INVALID"){
            this->ui->turn_ip_raise->setText(getParams(one_line_content,"set_bet_sizes ip,turn,raise").replace(',',' '));
        }
        else if(getParams(one_line_content,"set_bet_sizes ip,turn,allin") != "INVALID"){
            this->ui->turn_ip_allin->setChecked(true);
        }
        // RIVER
        else if(getParams(one_line_content,"set_bet_sizes oop,river,bet,") != "INVALID"){
            this->ui->river_oop_bet->setText(getParams(one_line_content,"set_bet_sizes oop,river,bet,").replace(',',' '));
        }
        else if(getParams(one_line_content,"set_bet_sizes oop,river,raise") != "INVALID"){
            this->ui->river_oop_raise->setText(getParams(one_line_content,"set_bet_sizes oop,river,raise").replace(',',' '));
        }
        else if(getParams(one_line_content,"set_bet_sizes oop,river,donk") != "INVALID"){
            this->ui->river_oop_donk->setText(getParams(one_line_content,"set_bet_sizes oop,river,donk").replace(',',' '));
        }
        else if(getParams(one_line_content,"set_bet_sizes oop,river,allin") != "INVALID"){
            this->ui->river_oop_allin->setChecked(true);
        }
        else if(getParams(one_line_content,"set_bet_sizes ip,river,bet,") != "INVALID"){
            this->ui->river_ip_bet->setText(getParams(one_line_content,"set_bet_sizes ip,river,bet,").replace(',',' '));
        }
        else if(getParams(one_line_content,"set_bet_sizes ip,river,raise") != "INVALID"){
            this->ui->river_ip_raise->setText(getParams(one_line_content,"set_bet_sizes ip,river,raise").replace(',',' '));
        }
        else if(getParams(one_line_content,"set_bet_sizes ip,river,allin") != "INVALID"){
            this->ui->river_ip_allin->setChecked(true);
        }
        // OTHER PARAMS
        else if(getParams(one_line_content,"set_allin_threshold") != "INVALID"){
            this->ui->allinThresholdText->setText(getParams(one_line_content,"set_allin_threshold"));
        }
        else if(getParams(one_line_content,"set_thread_num") != "INVALID"){
            this->ui->threadsText->setText(getParams(one_line_content,"set_thread_num"));
        }
        else if(getParams(one_line_content,"set_accuracy") != "INVALID"){
            this->ui->exploitabilityText->setText(getParams(one_line_content,"set_accuracy"));
        }
        else if(getParams(one_line_content,"set_max_iteration") != "INVALID"){
            this->ui->iterationText->setText(getParams(one_line_content,"set_max_iteration"));
        }
        else if(getParams(one_line_content,"set_print_interval") != "INVALID"){
            this->ui->logIntervalText->setText(getParams(one_line_content,"set_print_interval"));
        }
        else if(getParams(one_line_content,"set_raise_limit") != "INVALID"){
            this->ui->raiseLimitText->setText(getParams(one_line_content,"set_raise_limit"));
        }
        else if(getParams(one_line_content,"set_use_isomorphism") != "INVALID"){
            if(getParams(one_line_content,"set_use_isomorphism") == "1"){
                this->ui->useIsoCheck->setChecked(true);
            }else{
                this->ui->useIsoCheck->setChecked(false);
            }
        }
    }
    this->update();
}

void MainWindow::on_actionimport_triggered(){
    QString fileName =  QFileDialog::getOpenFileName(
              this,
              tr("Open parameters file"),
              QDir::currentPath(),
              tr("Text files (*.txt)"));
    this->import_from_file(fileName);
    this->ui->IpRangeTableView->update();
    this->ui->oopRangeTableView->update();
    this->ui->IpRangeTableView->setFocus();
    this->ui->oopRangeTableView->setFocus();
}

void MainWindow::on_wizardLoadConfigButton_clicked(){
    QString fileName =  QFileDialog::getOpenFileName(
              this,
              tr("Open parameters file"),
              QDir::currentPath(),
              tr("Text files (*.txt)"));
    this->import_from_file(fileName);
    this->ui->IpRangeTableView->update();
    this->ui->oopRangeTableView->update();
    this->ui->IpRangeTableView->setFocus();
    this->ui->oopRangeTableView->setFocus();
}

void MainWindow::on_actionexport_triggered(){
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Parameters"),
                               "parameters/output_parameters.txt",
                               tr("Text file (*.txt)"));
    if(fileName.isNull())return;
    QString output_text = "";
    QTextStream out(&output_text);
    out << "set_pot " << this->ui->potText->text().trimmed();
    out << "\n";
    out << "set_effective_stack " << this->ui->effectiveStackText->text().trimmed();
    out << "\n";
    out << "set_board " << this->ui->boardText->toPlainText();
    out << "\n";
    out << "set_range_oop " << this->ui->oopRangeText->toPlainText();
    out << "\n";
    out << "set_range_ip " << this->ui->ipRangeText->toPlainText();
    out << "\n";
    // FLOP
    out << "set_bet_sizes oop,flop,bet," << this->ui->flop_oop_bet->text().trimmed().replace(' ',',');
    out << "\n";
    out << "set_bet_sizes oop,flop,raise," << this->ui->flop_oop_raise->text().trimmed().replace(' ',',');
    out << "\n";
    if(this->ui->flop_ip_allin->isChecked()){
        out << "set_bet_sizes oop,flop,allin" << "\n";
    }
    out << "set_bet_sizes ip,flop,bet," << this->ui->flop_ip_bet->text().trimmed().replace(' ',',');
    out << "\n";
    out << "set_bet_sizes ip,flop,raise," << this->ui->flop_ip_raise->text().trimmed().replace(' ',',');
    out << "\n";
    if(this->ui->flop_ip_allin->isChecked()){
        out << "set_bet_sizes ip,flop,allin" << "\n";
    }
    // TURN
    out << "set_bet_sizes oop,turn,bet," << this->ui->turn_oop_bet->text().trimmed().replace(' ',',');
    out << "\n";
    out << "set_bet_sizes oop,turn,raise," << this->ui->turn_oop_raise->text().trimmed().replace(' ',',');
    out << "\n";
    if(this->ui->turn_oop_donk->text().trimmed() != ""){
        out << "set_bet_sizes oop,turn,donk," << this->ui->turn_oop_donk->text().trimmed().replace(' ',',');
        out << "\n";
    }
    if(this->ui->turn_ip_allin->isChecked()){
        out << "set_bet_sizes oop,turn,allin" << "\n";
    }
    out << "set_bet_sizes ip,turn,bet," << this->ui->turn_ip_bet->text().trimmed().replace(' ',',');
    out << "\n";
    out << "set_bet_sizes ip,turn,raise," << this->ui->turn_ip_raise->text().trimmed().replace(' ',',');
    out << "\n";
    if(this->ui->turn_ip_allin->isChecked()){
        out << "set_bet_sizes ip,turn,allin" << "\n";
    }
    // RIVER
    out << "set_bet_sizes oop,river,bet," << this->ui->river_oop_bet->text().trimmed().replace(' ',',');
    out << "\n";
    out << "set_bet_sizes oop,river,raise," << this->ui->river_oop_raise->text().trimmed().replace(' ',',');
    out << "\n";
    if(this->ui->river_ip_allin->isChecked()){
        out << "set_bet_sizes oop,river,allin" << "\n";
    }
    out << "set_bet_sizes ip,river,bet," << this->ui->river_ip_bet->text().trimmed().replace(' ',',');
    out << "\n";
    out << "set_bet_sizes ip,river,raise," << this->ui->river_ip_raise->text().trimmed().replace(' ',',');
    out << "\n";
    if(this->ui->river_oop_donk->text().trimmed() != ""){
        out << "set_bet_sizes oop,river,donk," << this->ui->river_oop_donk->text().trimmed().replace(' ',',');
        out << "\n";
    }
    if(this->ui->river_ip_allin->isChecked()){
        out << "set_bet_sizes ip,river,allin" << "\n";
    }

    out << "set_allin_threshold " << this->ui->allinThresholdText->text().trimmed();
    out << "\n";
    out << "set_raise_limit " << this->ui->raiseLimitText->text().trimmed();
    out << "\n";
    out << "build_tree";
    out << "\n";
    out << "set_thread_num " << this->ui->threadsText->text().trimmed();
    out << "\n";
    out << "set_accuracy " << this->ui->exploitabilityText->text().trimmed();
    out << "\n";
    out << "set_max_iteration " << this->ui->iterationText->text().trimmed();
    out << "\n";
    out << "set_print_interval " << this->ui->logIntervalText->text().trimmed();
    out << "\n";
    if(this->ui->useIsoCheck->isChecked()){
        out << "set_use_isomorphism 1" << "\n";
    }else{
        out << "set_use_isomorphism 0" << "\n";
    }
    out << "start_solve";
    out << "\n";

    this->setWindowTitle(tr("Settings"));
    QSettings setting("Solverix", "Setting");
    setting.beginGroup("solver");
    int dump_round = setting.value("dump_round").toInt();
    out << "set_dump_rounds " << dump_round;
    out << "\n";
    out << "dump_result output_result.json";

    setlocale(LC_ALL,"");

    ofstream fileWriter;
    fileWriter.open(fileName.toLocal8Bit());
    QMessageBox msgBox;
    QString message;
    if(!fileWriter.fail()){
        fileWriter << output_text.toStdString();
        fileWriter.flush();
        fileWriter.close();

         message = QObject::tr("save success");
    }else{
        message = QObject::tr("save failed, file cannot be open");
    }
    qDebug().noquote() << message;
    msgBox.setText(message);
    setlocale(LC_CTYPE, "C");
    msgBox.exec();
}

void MainWindow::on_actionSettings_triggered(){
    this->settingEditor = new SettingEditor(this);
    settingEditor->setAttribute(Qt::WA_DeleteOnClose);
    settingEditor->show();
}

void MainWindow::on_ip_range(QString range_text){
    this->ui->ipRangeText->setText(range_text);
}

void MainWindow::on_buttomSolve_clicked()
{   
    qSolverJob->max_iteration = ui->iterationText->text().toInt();
    qSolverJob->accuracy = ui->exploitabilityText->text().toFloat();
    qSolverJob->print_interval = ui->logIntervalText->text().toInt();
    qSolverJob->thread_number = ui->threadsText->text().toInt();
    qSolverJob->current_mission = QSolverJob::MissionType::SOLVING;
    this->solvingInProgress = true;
    qSolverJob->start();
}

Ui::MainWindow * MainWindow::getPriUi(){
    return this->ui;
}

QSTextEdit * MainWindow::getLogArea(){
    return this->ui->logOutput;
}

void MainWindow::on_clearLogButtom_clicked()
{
    this->ui->logOutput->clear();
}

vector<float> sizes_convert(QString input){
    QStringList list = input.split(" ");
    vector<float> sizes;
    foreach(QString num, list){
        if(num.endsWith("x")){
            int pos = num.lastIndexOf(QChar('x'));
            num = num.left(pos);
            sizes.push_back(num.toFloat()  * 100);
        }
        else{
            sizes.push_back(num.toFloat());
        }
    }
    return sizes;
}

void MainWindow::on_buildTreeButtom_clicked()
{
    if(!LicenseManager::isActive()){
        QMessageBox::warning(this, tr("Your subscription expired"),
            tr("Your Solverix subscription expired, so you can't keep solving. Reactivate it to continue."));
        LicenseDialog licenseDialog(this);
        licenseDialog.exec();
        if(!LicenseManager::isActive()) return;
    }
    qSolverJob->range_ip = this->ui->ipRangeText->toPlainText().toStdString();
    qSolverJob->range_oop = this->ui->oopRangeText->toPlainText().toStdString();
    qSolverJob->board = this->ui->boardText->toPlainText().toStdString();

    vector<string> board_str_arr = string_split(qSolverJob->board,',');
    if(board_str_arr.size() == 3){
        qSolverJob->current_round = 1;
    }else if(board_str_arr.size() == 4){
        qSolverJob->current_round = 2;
    }else if(board_str_arr.size() == 5){
        qSolverJob->current_round = 3;
    }else{
        this->ui->logOutput->log_with_signal(QString::fromStdString(tfm::format("Error : board %s not recognized",qSolverJob->board)));
        return;
    }
    qSolverJob->raise_limit = this->ui->raiseLimitText->text().toInt();
    qSolverJob->ip_commit = this->ui->potText->text().toFloat() / 2;
    qSolverJob->oop_commit = this->ui->potText->text().toFloat() / 2;
    qSolverJob->stack = this->ui->effectiveStackText->text().toFloat() + qSolverJob->ip_commit;
    qSolverJob->mode = this->ui->mode_box->currentIndex() == 0 ? QSolverJob::Mode::HOLDEM:QSolverJob::Mode::SHORTDECK;
    qSolverJob->allin_threshold = this->ui->allinThresholdText->text().toFloat();
    qSolverJob->use_isomorphism = this->ui->useIsoCheck->isChecked();
    qSolverJob->use_halffloats =  this->ui->useHalfFloats_box->currentIndex();

    StreetSetting gbs_flop_ip = StreetSetting(sizes_convert(ui->flop_ip_bet->text()),
                                              sizes_convert(ui->flop_ip_raise->text()),
                                              vector<float>{},
                                              ui->flop_ip_allin->isChecked()
                                              );
    StreetSetting gbs_turn_ip = StreetSetting(sizes_convert(ui->turn_ip_bet->text()),
                                              sizes_convert(ui->turn_ip_raise->text()),
                                              vector<float>{},
                                              ui->turn_ip_allin->isChecked()
                                              );
    StreetSetting gbs_river_ip = StreetSetting(sizes_convert(ui->river_ip_bet->text()),
                                              sizes_convert(ui->river_ip_raise->text()),
                                              vector<float>{},
                                              ui->river_ip_allin->isChecked()
                                              );

    StreetSetting gbs_flop_oop = StreetSetting(sizes_convert(ui->flop_oop_bet->text()),
                                              sizes_convert(ui->flop_oop_raise->text()),
                                              vector<float>{},
                                              ui->flop_oop_allin->isChecked()
                                              );
    StreetSetting gbs_turn_oop = StreetSetting(sizes_convert(ui->turn_oop_bet->text()),
                                              sizes_convert(ui->turn_oop_raise->text()),
                                              sizes_convert(ui->turn_oop_donk->text()),
                                              ui->turn_oop_allin->isChecked()
                                              );
    StreetSetting gbs_river_oop = StreetSetting(sizes_convert(ui->river_oop_bet->text()),
                                              sizes_convert(ui->river_oop_raise->text()),
                                              sizes_convert(ui->river_oop_donk->text()),
                                              ui->river_oop_allin->isChecked()
                                              );

    qSolverJob->gtbs = make_shared<GameTreeBuildingSettings>(gbs_flop_ip,gbs_turn_ip,gbs_river_ip,gbs_flop_oop,gbs_turn_oop,gbs_river_oop);
    qSolverJob->current_mission = QSolverJob::MissionType::BUILDTREE;
    qSolverJob->start();
}

void MainWindow::on_copyButtom_clicked()
{
    ui->flop_oop_bet->setText(ui->flop_ip_bet->text());
    ui->flop_oop_raise->setText(ui->flop_ip_raise->text());
    ui->flop_oop_allin->setChecked(ui->flop_ip_allin->isChecked());

    ui->turn_oop_bet->setText(ui->turn_ip_bet->text());
    ui->turn_oop_raise->setText(ui->turn_ip_raise->text());
    ui->turn_oop_allin->setChecked(ui->turn_ip_allin->isChecked());

    ui->river_oop_bet->setText(ui->river_ip_bet->text());
    ui->river_oop_raise->setText(ui->river_ip_raise->text());
    ui->river_oop_allin->setChecked(ui->river_ip_allin->isChecked());
}

void MainWindow::on_showResultButton_clicked()
{
    this->strategyExplorer = new StrategyExplorer(this,this->qSolverJob);
    strategyExplorer->setAttribute(Qt::WA_DeleteOnClose);
    strategyExplorer->show();
}

void MainWindow::on_stopSolvingButton_clicked()
{
    if(this->qSolverJob != NULL){
        this->qSolverJob->stop();
    }
}

void MainWindow::on_ipRangeSelectButtom_clicked()
{
    QSolverJob::Mode mode = this->ui->mode_box->currentIndex() == 0 ? QSolverJob::Mode::HOLDEM:QSolverJob::Mode::SHORTDECK;
    this->rangeSelector = new RangeSelector(this->ui->ipRangeText,this,mode);
    rangeSelector->setAttribute(Qt::WA_DeleteOnClose);
    rangeSelector->show();
}

void MainWindow::on_oopRangeSelectButtom_clicked()
{
    QSolverJob::Mode mode = this->ui->mode_box->currentIndex() == 0 ? QSolverJob::Mode::HOLDEM:QSolverJob::Mode::SHORTDECK;
    this->rangeSelector = new RangeSelector(this->ui->oopRangeText,this,mode);
    rangeSelector->setAttribute(Qt::WA_DeleteOnClose);
    rangeSelector->show();
}

float iso_corh(QString board){
    vector<string> board_str_arr = string_split(board.toStdString(),',');
    vector<Card> initialBoard;
    for(string one_board_str:board_str_arr){
        initialBoard.push_back(Card(one_board_str));
    }
    float corh = 1;
    uint16_t color_hash[4];
    for(int i = 0;i < 4;i ++)color_hash[i] = 0;
    for (Card one_card:initialBoard) {
        int rankind = one_card.getCardInt() % 4;
        int suitind = one_card.getCardInt() / 4;
        color_hash[rankind] = color_hash[rankind] | (1 << suitind);
    }
    for(int i = 0;i < 4;i ++){
        for(int j = 0;j < i;j ++){
            if(color_hash[i] == color_hash[j]){
                corh = corh * 0.70;
                continue;
            }
        }
    }
    return corh;
}

void MainWindow::on_estimateMemoryButtom_clicked()
{
    long long memory_float = this->qSolverJob->estimate_tree_memory(this->ui->ipRangeText->toPlainText(),this->ui->oopRangeText->toPlainText(),this->ui->boardText->toPlainText());
    // float32 should take 4bytes
    float corh = 1;
    if(this->ui->useIsoCheck->isChecked()){
        corh =iso_corh(this->ui->boardText->toPlainText());
    }
    switch(this->ui->useHalfFloats_box->currentIndex()){
    case 0:
        break;
    case 1:
        corh *= 0.75;
        break;
    case 2:
        corh *= 0.5;
        break;
    }
    float memory_mb = (float)memory_float / 1024 / 1024 * corh * 4 ;
    float memory_gb = (float)memory_float / 1024 / 1024 / 1024 * corh * 4;
    QString message;
    if(memory_gb == 0){
        message = tr("Please build tree first.");
    }else if(memory_gb < 1){
        message = tr("Estimated Memory Usage: ") + QString::number(memory_mb,'f',0) + tr(" Mb") +
                tr("\nRebuild tree to have changed optimization options take effect!");
    }else{
        message = tr("Estimated Memory Usage: ") + QString::number(memory_gb,'f',1) + tr(" Gb") +
                tr("\nRebuild tree to have changed optimization options take effect!");
    }
    qDebug().noquote() << message;
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.exec();
}

void MainWindow::on_selectBoardButton_clicked()
{
    QSolverJob::Mode mode = this->ui->mode_box->currentIndex() == 0 ? QSolverJob::Mode::HOLDEM:QSolverJob::Mode::SHORTDECK;
    this->boardSelector = new boardselector(this->ui->boardText,mode,this);
    boardSelector->setAttribute(Qt::WA_DeleteOnClose);
    boardSelector->show();
}

void MainWindow::on_actionopen_parameters_folder_triggered()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::current().filePath("parameters")));
}

void MainWindow::on_actionview_log_triggered()
{
    this->logDialog->show();
    this->logDialog->raise();
    this->logDialog->activateWindow();
}

void MainWindow::onSolverJobFinished()
{
    if(this->quickModePendingStage == QuickModeStage::WaitingForBuildTree){
        this->quickModePendingStage = QuickModeStage::WaitingForSolve;
        if(this->quickModeProgressDialog != NULL){
            this->quickModeProgressDialog->setLabelText(tr("Solving the optimal strategy... this can take a few seconds."));
        }
        this->on_buttomSolve_clicked();
        return;
    }
    if(this->quickModePendingStage == QuickModeStage::WaitingForSolve){
        this->quickModePendingStage = QuickModeStage::None;
        this->solvingInProgress = false;
        if(this->quickModeProgressDialog != NULL){
            this->quickModeProgressDialog->close();
            this->quickModeProgressDialog->deleteLater();
            this->quickModeProgressDialog = NULL;
        }
        this->strategyExplorer = new StrategyExplorer(this, this->qSolverJob);
        this->strategyExplorer->setAttribute(Qt::WA_DeleteOnClose);
        this->strategyExplorer->selectRootNode();
        this->strategyExplorer->setHighlightedHand(this->quickModeCard1, this->quickModeCard2);
        if(this->quizMode){
            this->quizMode = false;
            float foldPct, callPct, betPct;
            std::tie(foldPct, callPct, betPct) = this->strategyExplorer->getRootActionSummary();
            QString bestAction;
            float bestPct;
            if(foldPct >= callPct && foldPct >= betPct){ bestAction = "FOLD"; bestPct = foldPct; }
            else if(callPct >= foldPct && callPct >= betPct){ bestAction = "CALL"; bestPct = callPct; }
            else { bestAction = "BET"; bestPct = betPct; }
            QMap<QString,QString> actionNames;
            actionNames["FOLD"] = tr("Fold");
            actionNames["CALL"] = tr("Call / Check");
            actionNames["BET"] = tr("Bet / Raise");
            bool correct = (bestAction == this->quizGuessedAction);

            QDialog revealDialog(this);
            revealDialog.setWindowTitle(tr("Practice Mode"));
            revealDialog.setMinimumSize(560, 340);
            QVBoxLayout* layout = new QVBoxLayout(&revealDialog);
            layout->setSpacing(16);
            layout->setContentsMargins(28, 28, 28, 28);

            QLabel* resultLabel = new QLabel(correct ? tr("✅ You got it right!") : tr("❌ Not quite"), &revealDialog);
            resultLabel->setStyleSheet(correct
                ? "font-size:26px; font-weight:800; color:#22c55e;"
                : "font-size:26px; font-weight:800; color:#e05252;");
            layout->addWidget(resultLabel);

            QFrame* divider = new QFrame(&revealDialog);
            divider->setFrameShape(QFrame::HLine);
            layout->addWidget(divider);

            QLabel* guessLabel = new QLabel(tr("You said: <b>%1</b>").arg(actionNames[this->quizGuessedAction]), &revealDialog);
            guessLabel->setStyleSheet("font-size:15px;");
            layout->addWidget(guessLabel);

            QLabel* solverLabel = new QLabel(tr("With the whole range, the most frequent play here is: <b>%1 (%2%)</b>")
                .arg(actionNames[bestAction]).arg((int)(bestPct * 100 + 0.5f)), &revealDialog);
            solverLabel->setWordWrap(true);
            solverLabel->setStyleSheet("font-size:15px;");
            layout->addWidget(solverLabel);

            layout->addStretch();

            QLabel* hintLabel = new QLabel(correct
                ? tr("Nice! Explore below to see the hand-by-hand detail.")
                : tr("No worries, check the hand-by-hand detail below to understand why."), &revealDialog);
            hintLabel->setWordWrap(true);
            hintLabel->setStyleSheet("font-size:13px; color:#a9c9b6;");
            layout->addWidget(hintLabel);

            QPushButton* closeBtn = new QPushButton(tr("See the detail →"), &revealDialog);
            closeBtn->setStyleSheet("font-size:15px; font-weight:700; padding:12px 10px;");
            connect(closeBtn, &QPushButton::clicked, &revealDialog, &QDialog::accept);
            layout->addWidget(closeBtn);

            revealDialog.exec();
            this->strategyExplorer->show();
        }else{
            // Embed the results directly into step 4 of the quick-mode
            // wizard instead of opening a separate floating window, so the
            // user stays on one screen and can start a new hand right there.
            this->strategyExplorer->setAttribute(Qt::WA_DeleteOnClose, false);
            this->strategyExplorer->setWindowFlags(Qt::Widget);
            this->strategyExplorer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            this->ui->quickResultsContainer->layout()->addWidget(this->strategyExplorer);
            this->strategyExplorer->show();
            this->showQuickModeStep(3);
        }
        return;
    }
    if(this->solvingInProgress){
        this->solvingInProgress = false;
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Done"));
        msgBox.setText(tr("The solver finished calculating the strategy. Tap \"ShowResult\" to see it."));
        msgBox.exec();
    }
}

void MainWindow::setupQuickSeatButtons(int tableSize){
    this->quickTableSize = tableSize;
    QStringList preflopOrder = getTableSeatOrder(tableSize);

    // Visual placement clockwise from the top, starting at BB (matches the
    // original 5-seat layout's convention). Distinct from preflop acting
    // order, which is only used to compute who's the opener.
    QStringList displayOrder;
    displayOrder << "BB";
    for(const QString& seat : preflopOrder){
        if(seat != "BB") displayOrder << seat;
    }
    this->quickSeatDisplayOrder = displayOrder;

    QWidget* container = this->ui->seatTableContainer;
    double centerX = container->width() / 2.0;
    double centerY = container->height() / 2.0;
    double radiusX = centerX - 60;
    double radiusY = centerY - 40;
    const int btnW = 80, btnH = 50;

    while(this->quickSeatButtons.size() < 9){
        QPushButton* btn = new QPushButton(container);
        btn->setMinimumSize(btnW, btnH);
        btn->setMaximumSize(btnW, btnH);
        connect(btn, &QPushButton::clicked, this, &MainWindow::onSeatClicked);
        this->quickSeatButtons.append(btn);
    }

    for(int i = 0; i < this->quickSeatButtons.size(); i++){
        QPushButton* btn = this->quickSeatButtons[i];
        if(i < displayOrder.size()){
            QString seat = displayOrder[i];
            double angle = -M_PI/2.0 + (2.0 * M_PI * i / displayOrder.size());
            int x = (int)(centerX + radiusX * cos(angle) - btnW/2.0);
            int y = (int)(centerY + radiusY * sin(angle) - btnH/2.0);
            btn->setObjectName("quickSeat_" + seat);
            btn->setGeometry(x, y, btnW, btnH);
            btn->setText(seat);
            btn->setVisible(true);
            btn->raise();
        }else{
            btn->setVisible(false);
        }
    }
}

void MainWindow::onTableSizeChosen(int tableSize){
    this->setupQuickSeatButtons(tableSize);
    this->resetSeatSelection();
}

void MainWindow::onTableSize6Clicked(){ this->onTableSizeChosen(6); }
void MainWindow::onTableSize9Clicked(){ this->onTableSizeChosen(9); }

void MainWindow::updateSeatButtonStyles(){
    for(QPushButton* button : this->quickSeatButtons){
        if(!button->isVisible()) continue;
        QString seat = button->objectName().mid(QString("quickSeat_").length());
        if(seat == this->mySeat){
            button->setText(seat + "\n" + tr("(YOU)"));
        }else if(seat == this->villainSeat){
            button->setText(seat + "\n" + tr("(OPPONENT)"));
        }else{
            button->setText(seat);
        }
    }
}

void MainWindow::resetSeatSelection(){
    this->mySeat = "";
    this->villainSeat = "";
    this->ui->seatSelectionStatusLabel->setText(tr("Choose your seat."));
    this->updateSeatButtonStyles();
}

void MainWindow::onSeatClicked(){
    QPushButton* clicked = qobject_cast<QPushButton*>(sender());
    if(clicked == NULL) return;
    QString seat = clicked->objectName().mid(QString("quickSeat_").length());

    if(this->mySeat.isEmpty()){
        this->mySeat = seat;
        this->ui->seatSelectionStatusLabel->setText(tr("Now choose your opponent's seat."));
        this->updateSeatButtonStyles();
        return;
    }
    if(seat == this->mySeat){
        this->resetSeatSelection();
        return;
    }
    if(!this->villainSeat.isEmpty()) return;

    this->villainSeat = seat;
    this->updateSeatButtonStyles();

    QStringList preflopOrder = getTableSeatOrder(this->quickTableSize);
    QString openerSeat = preflopOrder.indexOf(this->mySeat) < preflopOrder.indexOf(this->villainSeat) ? this->mySeat : this->villainSeat;
    QString callerSeat = (openerSeat == this->mySeat) ? this->villainSeat : this->mySeat;
    this->currentMatchup = buildQuickModeMatchup(this->quickTableSize, openerSeat, callerSeat);
    this->userIsOpener = (this->mySeat == openerSeat);

    this->handSelectorModel->clear_board();
    this->ui->handSelectorTable->update();
    this->ui->handSelectedLabel->setText(tr("Selected: (none)"));
    this->showQuickModeStep(1);
}

void MainWindow::onHandSelectorClicked(const QModelIndex &index){
    int row = index.row();
    int col = index.column();
    bool wasSelected = this->handSelectorModel->getBoardAt(row, col) > 0;
    if(!wasSelected){
        int selectedCount = 0;
        for(int i = 0; i < 4; i++){
            for(int j = 0; j < 13; j++){
                if(this->handSelectorModel->getBoardAt(i, j) > 0) selectedCount++;
            }
        }
        if(selectedCount >= 2){
            QMessageBox::information(this, tr("You already chose 2 cards"),
                tr("You can only choose 2 cards for your hand. Tap one of the already selected ones to remove it."));
            return;
        }
    }
    this->handSelectorModel->setBoardAt(row, col, wasSelected ? 0 : 1);
    this->ui->handSelectorTable->update();
    QString text = this->handSelectorModel->getBoardText();
    this->ui->handSelectedLabel->setText(text.isEmpty() ? tr("Selected: (none)") : tr("Selected: %1").arg(text));
}

void MainWindow::onSubscriptionCheckTimer(){
    QString email = LicenseManager::currentUserEmail();
    if(email.isEmpty()) return;

    bool wasActive = LicenseManager::isActive();
    ApiClient::checkStatus(email, [this, wasActive](ApiClient::LoginResult result){
        if(!result.ok) return; // network hiccup or similar — keep the cached value, try again next hour

        LicenseManager::cacheFromServer(result.plan, result.expiresAt);

        if(wasActive && !result.active){
            QMessageBox::warning(this, tr("Your subscription expired"),
                tr("Your Solverix subscription expired or was cancelled. You'll need to reactivate it to keep solving hands."));
        }
    });
}

void MainWindow::onNewHandButtonClicked(){
    if(this->strategyExplorer != NULL){
        this->ui->quickResultsContainer->layout()->removeWidget(this->strategyExplorer);
        this->strategyExplorer->deleteLater();
        this->strategyExplorer = NULL;
    }
    this->startQuickMode();
}

void MainWindow::startQuickMode(){
    this->quickMode = true;
    this->exampleMode = false;
    for(int i = 0; i < 6; i++){
        this->wizardSteps[i]->setVisible(false);
    }
    this->handSelectorModel->clear_board();
    this->ui->handSelectedLabel->setText(tr("Selected: (none)"));
    this->setupQuickSeatButtons(6);
    this->resetSeatSelection();
    this->showQuickModeStep(0);
}

void MainWindow::showQuickModeStep(int index){
    if(index < 0 || index > 3) return;
    this->currentQuickStep = index;
    for(int i = 0; i < 4; i++){
        this->quickModeSteps[i]->setVisible(i == index);
    }
    QStringList stepNames;
    stepNames << tr("Situation") << tr("Your cards") << tr("Board") << tr("Result");
    this->ui->wizardStepLabel->setText(tr("Quick Mode — Step %1 of 4: %2").arg(index + 1).arg(stepNames[index]));
    this->ui->wizardStepSubtitle->setText("");
    this->ui->exampleBanner->setVisible(false);
    // Step 3 (results) has its own "nueva mano" control; the generic
    // back/next nav doesn't apply there.
    bool showNav = (index < 3);
    this->ui->wizardBackButton->setVisible(showNav);
    this->ui->wizardNextButton->setVisible(showNav);
    this->ui->wizardBackButton->setEnabled(index > 0);
    this->ui->wizardNextButton->setText(index == 2 ? tr("Solve →") : tr("Next →"));

    // The results step should fill all the leftover vertical space itself
    // instead of leaving it to the trailing spacer (item 10) below, which
    // is what every other step lets absorb the extra room.
    QBoxLayout* inputLayout = qobject_cast<QBoxLayout*>(this->ui->inputLayout);
    if(inputLayout != NULL){
        inputLayout->setStretch(9, index == 3 ? 1 : 0);
        inputLayout->setStretch(10, index == 3 ? 0 : 1);
    }
}

void MainWindow::startQuickModeSolve(bool fastMode){
    QuickModeMatchup matchup = this->currentMatchup;
    if(matchup.openerRange.isEmpty() || matchup.callerRange.isEmpty()){
        QMessageBox::warning(this, tr("You need to choose the situation"),
            tr("No valid situation (seats) was chosen before solving. Go back to step 1 and choose your seat and your opponent's."));
        this->showQuickModeStep(0);
        return;
    }

    QString myRange = this->userIsOpener ? matchup.openerRange : matchup.callerRange;
    QString villainRange = this->userIsOpener ? matchup.callerRange : matchup.openerRange;
    bool myRangeIsIP = this->userIsOpener ? matchup.openerIsIP : !matchup.openerIsIP;

    if(myRangeIsIP){
        this->ui->ipRangeText->setPlainText(myRange);
        this->ui->oopRangeText->setPlainText(villainRange);
    }else{
        this->ui->ipRangeText->setPlainText(villainRange);
        this->ui->oopRangeText->setPlainText(myRange);
    }

    this->ui->potText->setText(QString::number(matchup.pot, 'f', 1));
    this->ui->effectiveStackText->setText(QString::number(matchup.effectiveStack, 'f', 1));
    this->ui->allinThresholdText->setText("0.67");
    this->ui->useIsoCheck->setChecked(true);
    this->ui->useHalfFloats_box->setCurrentIndex(0); // half-floats is slower here, not faster
    this->ui->mode_box->setCurrentIndex(0);

    // These all come from Configuración now, so the speed/precision
    // trade-off is user-tunable instead of hardcoded.
    QSettings quickSettings("Solverix", "Setting");
    quickSettings.beginGroup("quickmode");
    int iterations = quickSettings.value(fastMode ? "quizIterations" : "iterations", fastMode ? 25 : 35).toInt();
    double exploitability = quickSettings.value("exploitability", 4.0).toDouble();
    int logInterval = quickSettings.value("logInterval", 10).toInt();
    int threads = quickSettings.value("threads", 8).toInt();
    int raiseLimit = quickSettings.value("raiseLimit", 1).toInt();
    quickSettings.endGroup();

    this->ui->raiseLimitText->setText(QString::number(raiseLimit));
    this->ui->iterationText->setText(QString::number(iterations));
    this->ui->exploitabilityText->setText(QString::number(exploitability, 'f', 1));
    this->ui->logIntervalText->setText(QString::number(logInterval));
    this->ui->threadsText->setText(QString::number(threads));

    QStringList betSizeFields = {"flop_ip_bet","turn_ip_bet","river_ip_bet","flop_oop_bet","turn_oop_bet","river_oop_bet"};
    QStringList raiseSizeFields = {"flop_ip_raise","turn_ip_raise","river_ip_raise","flop_oop_raise","turn_oop_raise","river_oop_raise"};
    for(const QString& name : betSizeFields){
        QLineEdit* field = this->findChild<QLineEdit*>(name);
        if(field != NULL) field->setText("50");
    }
    for(const QString& name : raiseSizeFields){
        QLineEdit* field = this->findChild<QLineEdit*>(name);
        if(field != NULL) field->setText("60");
    }
    QStringList allinChecks = {"flop_ip_allin","turn_ip_allin","river_ip_allin","flop_oop_allin","turn_oop_allin","river_oop_allin"};
    for(const QString& name : allinChecks){
        QCheckBox* field = this->findChild<QCheckBox*>(name);
        if(field != NULL) field->setChecked(false);
    }
    QLineEdit* turnDonk = this->findChild<QLineEdit*>("turn_oop_donk");
    if(turnDonk != NULL) turnDonk->setText("");
    QLineEdit* riverDonk = this->findChild<QLineEdit*>("river_oop_donk");
    if(riverDonk != NULL) riverDonk->setText("");

    this->quickModeProgressDialog = new QProgressDialog(this);
    this->quickModeProgressDialog->setWindowTitle(tr("Solving"));
    this->quickModeProgressDialog->setLabelText(tr("Building the decision tree..."));
    this->quickModeProgressDialog->setRange(0, 0); // indeterminate (busy) bar
    this->quickModeProgressDialog->setCancelButton(NULL);
    this->quickModeProgressDialog->setWindowModality(Qt::WindowModal);
    this->quickModeProgressDialog->setMinimumDuration(0);
    this->quickModeProgressDialog->show();

    this->quickModePendingStage = QuickModeStage::WaitingForBuildTree;
    this->on_buildTreeButtom_clicked();
}

void MainWindow::on_ipRangeText_textChanged()
{
    this->ip_model->setRangeText(this->ui->ipRangeText->toPlainText());
    this->ui->IpRangeTableView->update();
}

void MainWindow::on_oopRangeText_textChanged()
{
    this->oop_model->setRangeText(this->ui->oopRangeText->toPlainText());
    this->ui->oopRangeTableView->update();
}

