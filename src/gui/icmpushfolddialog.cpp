#include "include/gui/icmpushfolddialog.h"
#include "include/data/icmcalculator.h"
#include "include/data/preflopequity.h"
#include "include/gui/boardselector.h"
#include "include/runtime/qsolverjob.h"
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QSpinBox>
#include <QFormLayout>
#include <QScrollArea>
#include <QMessageBox>
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QApplication>
#include <QHeaderView>
#include <QFrame>

// Renders a card string like "Jh" as a small white "chip" (rank + colored
// suit symbol on a light background), matching the card-chip motif already
// used on the marketing site's hero banner mockup -- reads the same
// regardless of which of the 5 app themes is active, since it's not
// QSS-driven.
static void styleCardChip(QLabel* chip, const QString& card){
    if(card.length() != 2) return;
    QString rank = card.left(1);
    QChar suitChar = card.at(1);
    QString suitSymbol;
    QString suitColor = "#1a1d29";
    if(suitChar == 'h'){ suitSymbol = "♥"; suitColor = "#d1352b"; }
    else if(suitChar == 'd'){ suitSymbol = "♦"; suitColor = "#d1352b"; }
    else if(suitChar == 'c'){ suitSymbol = "♣"; }
    else if(suitChar == 's'){ suitSymbol = "♠"; }
    chip->setText(QString("<div style=\"text-align:center;\">%1<br>%2</div>").arg(rank, suitSymbol));
    chip->setStyleSheet(QString(
        "background:#f5f2e8; color:%1; border-radius:6px;"
        "font-size:18px; font-weight:700; font-family:'Segoe UI',sans-serif;"
    ).arg(suitColor));
}

IcmPushFoldDialog::IcmPushFoldDialog(std::shared_ptr<Compairer> compairer, QWidget* parent)
    : QDialog(parent), compairer(compairer)
{
    this->setWindowTitle(tr("Tournament Games Pre-Flop"));
    this->resize(480, 560);

    QVBoxLayout* outer = new QVBoxLayout(this);
    this->stack = new QStackedWidget(this);
    outer->addWidget(this->stack);

    // ---- Page 0: scenario ----
    QWidget* scenarioPage = new QWidget(this);
    QVBoxLayout* scenarioLayout = new QVBoxLayout(scenarioPage);
    scenarioLayout->addWidget(new QLabel(tr("<b>What happened?</b>"), scenarioPage));
    QRadioButton* openShoveRadio = new QRadioButton(tr("Someone shoved all-in first (no raise before it)"), scenarioPage);
    QRadioButton* reshoveRadio = new QRadioButton(tr("I raised and got shoved on"), scenarioPage);
    openShoveRadio->setChecked(true);
    QButtonGroup* scenarioGroup = new QButtonGroup(scenarioPage);
    scenarioGroup->addButton(openShoveRadio);
    scenarioGroup->addButton(reshoveRadio);
    scenarioLayout->addWidget(openShoveRadio);
    scenarioLayout->addWidget(reshoveRadio);
    scenarioLayout->addStretch();
    QPushButton* scenarioNext = new QPushButton(tr("Next →"), scenarioPage);
    scenarioLayout->addWidget(scenarioNext);
    connect(scenarioNext, &QPushButton::clicked, this, [this, openShoveRadio](){
        this->scenario = openShoveRadio->isChecked()
            ? PushFoldScenario::OpenShove : PushFoldScenario::ReshoveOverOpen;
        this->onScenarioContinue();
    });
    this->stack->addWidget(scenarioPage);

    // ---- Page 1: how many players ----
    QWidget* countPage = new QWidget(this);
    QVBoxLayout* countPageLayout = new QVBoxLayout(countPage);
    countPageLayout->addWidget(this->buildProgressDots(0));
    QLabel* countTitle = new QLabel(tr("<h3>How many players are<br>left at the table?</h3>"), countPage);
    countTitle->setAlignment(Qt::AlignCenter);
    countPageLayout->addWidget(countTitle);
    countPageLayout->addStretch();
    countPageLayout->addWidget(this->buildStepperRow(&this->playerCount, 2, 9, &this->playerCountLabel, [](){}));
    countPageLayout->addStretch();
    QPushButton* countBack = new QPushButton(tr("← Back"), countPage);
    countBack->setObjectName("wizardBackButton");
    QPushButton* countNext = new QPushButton(tr("Next →"), countPage);
    QHBoxLayout* countNav = new QHBoxLayout();
    countNav->addWidget(countBack);
    countNav->addWidget(countNext);
    countPageLayout->addLayout(countNav);
    connect(countBack, &QPushButton::clicked, this, &IcmPushFoldDialog::onBack);
    connect(countNext, &QPushButton::clicked, this, [this](){ this->goToPage(2); });
    this->stack->addWidget(countPage);

    // ---- Page 2: your stack and your opponent's ----
    QWidget* heroVillainPage = new QWidget(this);
    QVBoxLayout* heroVillainLayout = new QVBoxLayout(heroVillainPage);
    heroVillainLayout->addWidget(this->buildProgressDots(1));
    QLabel* heroVillainTitle = new QLabel(tr("<h3>How many chips do you<br>and your opponent have?</h3>"), heroVillainPage);
    heroVillainTitle->setAlignment(Qt::AlignCenter);
    heroVillainLayout->addWidget(heroVillainTitle);
    heroVillainLayout->addStretch();

    QHBoxLayout* heroVillainCards = new QHBoxLayout();
    QWidget* heroCard = new QWidget(heroVillainPage);
    heroCard->setObjectName("icmStepCardHighlight");
    QVBoxLayout* heroCardLayout = new QVBoxLayout(heroCard);
    QLabel* heroTag = new QLabel(tr("YOU"), heroCard);
    heroTag->setAlignment(Qt::AlignCenter);
    this->heroStackInput = new QLineEdit(heroCard);
    this->heroStackInput->setValidator(new QDoubleValidator(0, 100000000, 0, this->heroStackInput));
    this->heroStackInput->setAlignment(Qt::AlignCenter);
    this->heroStackInput->setPlaceholderText(tr("chips"));
    heroCardLayout->addWidget(heroTag);
    heroCardLayout->addWidget(this->heroStackInput);

    QWidget* villainCard = new QWidget(heroVillainPage);
    villainCard->setObjectName("icmStepCard");
    QVBoxLayout* villainCardLayout = new QVBoxLayout(villainCard);
    QLabel* villainTag = new QLabel(tr("OPPONENT"), villainCard);
    villainTag->setAlignment(Qt::AlignCenter);
    this->villainStackInput = new QLineEdit(villainCard);
    this->villainStackInput->setValidator(new QDoubleValidator(0, 100000000, 0, this->villainStackInput));
    this->villainStackInput->setAlignment(Qt::AlignCenter);
    this->villainStackInput->setPlaceholderText(tr("chips"));
    villainCardLayout->addWidget(villainTag);
    villainCardLayout->addWidget(this->villainStackInput);

    heroVillainCards->addWidget(heroCard);
    heroVillainCards->addWidget(villainCard);
    heroVillainLayout->addLayout(heroVillainCards);
    heroVillainLayout->addStretch();

    QPushButton* heroVillainBack = new QPushButton(tr("← Back"), heroVillainPage);
    heroVillainBack->setObjectName("wizardBackButton");
    QPushButton* heroVillainNext = new QPushButton(tr("Next →"), heroVillainPage);
    QHBoxLayout* heroVillainNav = new QHBoxLayout();
    heroVillainNav->addWidget(heroVillainBack);
    heroVillainNav->addWidget(heroVillainNext);
    heroVillainLayout->addLayout(heroVillainNav);
    connect(heroVillainBack, &QPushButton::clicked, this, &IcmPushFoldDialog::onBack);
    connect(heroVillainNext, &QPushButton::clicked, this, &IcmPushFoldDialog::onHeroVillainContinue);
    this->stack->addWidget(heroVillainPage);

    // ---- Page 3: the rest of the table (skipped if playerCount == 2) ----
    QWidget* othersPage = new QWidget(this);
    QVBoxLayout* othersLayout = new QVBoxLayout(othersPage);
    othersLayout->addWidget(this->buildProgressDots(2));
    QLabel* othersTitle = new QLabel(tr("<h3>How many chips does<br>everyone else have?</h3>"), othersPage);
    othersTitle->setAlignment(Qt::AlignCenter);
    othersLayout->addWidget(othersTitle);

    QScrollArea* othersScroll = new QScrollArea(othersPage);
    othersScroll->setWidgetResizable(true);
    QWidget* othersRowsWidget = new QWidget(othersScroll);
    this->otherPlayersLayout = new QVBoxLayout(othersRowsWidget);
    othersScroll->setWidget(othersRowsWidget);
    othersLayout->addWidget(othersScroll);

    QPushButton* othersBack = new QPushButton(tr("← Back"), othersPage);
    othersBack->setObjectName("wizardBackButton");
    QPushButton* othersNext = new QPushButton(tr("Next →"), othersPage);
    QHBoxLayout* othersNav = new QHBoxLayout();
    othersNav->addWidget(othersBack);
    othersNav->addWidget(othersNext);
    othersLayout->addLayout(othersNav);
    connect(othersBack, &QPushButton::clicked, this, &IcmPushFoldDialog::onBack);
    connect(othersNext, &QPushButton::clicked, this, &IcmPushFoldDialog::onOtherPlayersContinue);
    this->stack->addWidget(othersPage);

    // ---- Page 4: big blind ----
    QWidget* blindPage = new QWidget(this);
    QVBoxLayout* blindPageLayout = new QVBoxLayout(blindPage);
    blindPageLayout->addWidget(this->buildProgressDots(3));
    QLabel* blindTitle = new QLabel(tr("<h3>What's the current<br>big blind?</h3>"), blindPage);
    blindTitle->setAlignment(Qt::AlignCenter);
    blindPageLayout->addWidget(blindTitle);
    blindPageLayout->addStretch();
    this->bigBlindInput = new QLineEdit(blindPage);
    this->bigBlindInput->setValidator(new QDoubleValidator(0.01, 1000000, 2, this->bigBlindInput));
    this->bigBlindInput->setAlignment(Qt::AlignCenter);
    this->bigBlindInput->setPlaceholderText(tr("chips"));
    this->bigBlindInput->setStyleSheet("font-size:26px; font-weight:800; padding:14px;");
    blindPageLayout->addWidget(this->bigBlindInput);
    blindPageLayout->addStretch();
    QPushButton* blindBack = new QPushButton(tr("← Back"), blindPage);
    blindBack->setObjectName("wizardBackButton");
    QPushButton* blindNext = new QPushButton(tr("Next →"), blindPage);
    QHBoxLayout* blindNav = new QHBoxLayout();
    blindNav->addWidget(blindBack);
    blindNav->addWidget(blindNext);
    blindPageLayout->addLayout(blindNav);
    connect(blindBack, &QPushButton::clicked, this, &IcmPushFoldDialog::onBack);
    connect(blindNext, &QPushButton::clicked, this, &IcmPushFoldDialog::onBigBlindContinue);
    this->stack->addWidget(blindPage);

    // ---- Page 5: payouts ----
    QWidget* payoutsPage = new QWidget(this);
    QVBoxLayout* payoutsPageLayout = new QVBoxLayout(payoutsPage);
    payoutsPageLayout->addWidget(this->buildProgressDots(4));
    QLabel* payoutsTitle = new QLabel(tr("<h3>How much does each<br>place pay?</h3>"), payoutsPage);
    payoutsTitle->setAlignment(Qt::AlignCenter);
    payoutsPageLayout->addWidget(payoutsTitle);
    payoutsPageLayout->addWidget(this->buildStepperRow(&this->payoutCount, 1, 9, &this->payoutCountLabel,
        [this](){ this->rebuildPayoutRows(); }));

    QScrollArea* payoutScroll = new QScrollArea(payoutsPage);
    payoutScroll->setWidgetResizable(true);
    QWidget* payoutRowsWidget = new QWidget(payoutScroll);
    this->payoutRowsLayout = new QVBoxLayout(payoutRowsWidget);
    payoutScroll->setWidget(payoutRowsWidget);
    payoutsPageLayout->addWidget(payoutScroll);

    QPushButton* payoutsBack = new QPushButton(tr("← Back"), payoutsPage);
    payoutsBack->setObjectName("wizardBackButton");
    QPushButton* payoutsNext = new QPushButton(tr("Next →"), payoutsPage);
    QHBoxLayout* payoutsNav = new QHBoxLayout();
    payoutsNav->addWidget(payoutsBack);
    payoutsNav->addWidget(payoutsNext);
    payoutsPageLayout->addLayout(payoutsNav);
    connect(payoutsBack, &QPushButton::clicked, this, &IcmPushFoldDialog::onBack);
    connect(payoutsNext, &QPushButton::clicked, this, &IcmPushFoldDialog::onPayoutsContinue);
    this->stack->addWidget(payoutsPage);
    this->rebuildPayoutRows();

    // ---- Page 6: hand ----
    QWidget* handPage = new QWidget(this);
    QVBoxLayout* handPageLayout = new QVBoxLayout(handPage);
    handPageLayout->addWidget(new QLabel(tr("<b>Your hand</b>"), handPage));
    this->handTextEdit = new QTextEdit(handPage);
    this->handTextEdit->setVisible(false); // used only as boardselector's storage target
    this->handLabel = new QLabel(tr("(not chosen)"), handPage);
    handPageLayout->addWidget(this->handLabel);
    QPushButton* pickHandButton = new QPushButton(tr("Choose your hand"), handPage);
    handPageLayout->addWidget(pickHandButton);
    connect(pickHandButton, &QPushButton::clicked, this, &IcmPushFoldDialog::onPickHandClicked);
    handPageLayout->addStretch();

    QPushButton* handBack = new QPushButton(tr("← Back"), handPage);
    handBack->setObjectName("wizardBackButton");
    QPushButton* handNext = new QPushButton(tr("See result →"), handPage);
    QHBoxLayout* handNav = new QHBoxLayout();
    handNav->addWidget(handBack);
    handNav->addWidget(handNext);
    handPageLayout->addLayout(handNav);
    connect(handBack, &QPushButton::clicked, this, &IcmPushFoldDialog::onBack);
    connect(handNext, &QPushButton::clicked, this, [this](){
        QStringList cards = this->handTextEdit->toPlainText().split(",", Qt::SkipEmptyParts);
        if(cards.size() != 2){
            QMessageBox::information(this, tr("Choose your hand"), tr("You need to choose 2 cards before continuing."));
            return;
        }
        this->showResult();
    });
    this->stack->addWidget(handPage);

    // ---- Page 7: result ----
    QWidget* resultPage = new QWidget(this);
    QVBoxLayout* resultPageLayout = new QVBoxLayout(resultPage);

    QHBoxLayout* handCardsLayout = new QHBoxLayout();
    handCardsLayout->addStretch();
    this->handCard1Chip = new QLabel(resultPage);
    this->handCard2Chip = new QLabel(resultPage);
    for(QLabel* chip : { this->handCard1Chip, this->handCard2Chip }){
        chip->setFixedSize(52, 70);
        chip->setAlignment(Qt::AlignCenter);
    }
    handCardsLayout->addWidget(this->handCard1Chip);
    handCardsLayout->addWidget(this->handCard2Chip);
    handCardsLayout->addStretch();
    resultPageLayout->addLayout(handCardsLayout);

    this->resultLabel = new QLabel(resultPage);
    this->resultLabel->setObjectName("icmResultBanner");
    this->resultLabel->setAlignment(Qt::AlignCenter);
    resultPageLayout->addWidget(this->resultLabel);

    QHBoxLayout* statRow = new QHBoxLayout();
    this->evCallBox = new QLabel(resultPage);
    this->evFoldBox = new QLabel(resultPage);
    for(QLabel* box : { this->evCallBox, this->evFoldBox }){
        box->setAlignment(Qt::AlignCenter);
        box->setWordWrap(true);
    }
    statRow->addWidget(this->evCallBox);
    statRow->addWidget(this->evFoldBox);
    resultPageLayout->addLayout(statRow);

    resultPageLayout->addStretch();

    QPushButton* resultBack = new QPushButton(tr("← Back"), resultPage);
    resultBack->setObjectName("wizardBackButton");
    connect(resultBack, &QPushButton::clicked, this, &IcmPushFoldDialog::onBack);
    resultPageLayout->addWidget(resultBack);
    this->stack->addWidget(resultPage);
}

QWidget* IcmPushFoldDialog::buildProgressDots(int currentStep){
    QWidget* row = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(row);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(6);
    for(int i = 0; i < 5; i++){
        QFrame* dot = new QFrame(row);
        dot->setObjectName(i <= currentStep ? "icmProgressDotOn" : "icmProgressDotOff");
        dot->setFixedSize(22, 4);
        layout->addWidget(dot);
    }
    return row;
}

QWidget* IcmPushFoldDialog::buildStepperRow(int* value, int minValue, int maxValue, QLabel** outValueLabel, std::function<void()> onChange){
    QWidget* row = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(row);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);

    QPushButton* minusButton = new QPushButton("−", row);
    minusButton->setFixedSize(38, 38);

    QLabel* valueLabel = new QLabel(QString::number(*value), row);
    valueLabel->setAlignment(Qt::AlignCenter);
    valueLabel->setMinimumWidth(50);
    valueLabel->setStyleSheet("font-size:32px; font-weight:800;");
    *outValueLabel = valueLabel;

    QPushButton* plusButton = new QPushButton("+", row);
    plusButton->setFixedSize(38, 38);

    connect(minusButton, &QPushButton::clicked, this, [value, minValue, valueLabel, onChange](){
        if(*value > minValue){
            (*value)--;
            valueLabel->setText(QString::number(*value));
            onChange();
        }
    });
    connect(plusButton, &QPushButton::clicked, this, [value, maxValue, valueLabel, onChange](){
        if(*value < maxValue){
            (*value)++;
            valueLabel->setText(QString::number(*value));
            onChange();
        }
    });

    layout->addWidget(minusButton);
    layout->addWidget(valueLabel);
    layout->addWidget(plusButton);
    return row;
}

void IcmPushFoldDialog::goToPage(int index){
    this->navHistory.append(this->stack->currentIndex());
    this->stack->setCurrentIndex(index);
}

void IcmPushFoldDialog::onScenarioContinue(){
    this->goToPage(1);
}

void IcmPushFoldDialog::onHeroVillainContinue(){
    if(this->heroStackInput->text().trimmed().isEmpty() || this->villainStackInput->text().trimmed().isEmpty()){
        QMessageBox::information(this, tr("Missing data"), tr("Enter both stacks before continuing."));
        return;
    }
    if(this->playerCount > 2){
        this->rebuildOtherPlayerRows();
        this->goToPage(3);
    }else{
        this->goToPage(4);
    }
}

void IcmPushFoldDialog::rebuildOtherPlayerRows(){
    QLayoutItem* child;
    while((child = this->otherPlayersLayout->takeAt(0)) != nullptr){
        if(child->widget()) child->widget()->deleteLater();
        delete child;
    }
    this->otherPlayerInputs.clear();

    for(int i = 0; i < this->playerCount - 2; i++){
        QWidget* row = new QWidget();
        row->setObjectName("icmStepCard");
        QHBoxLayout* rowLayout = new QHBoxLayout(row);
        rowLayout->addWidget(new QLabel(tr("Player %1:").arg(i + 3)));
        QLineEdit* stackEdit = new QLineEdit(row);
        stackEdit->setValidator(new QDoubleValidator(0, 100000000, 0, stackEdit));
        stackEdit->setPlaceholderText(tr("chips"));
        rowLayout->addWidget(stackEdit);
        this->otherPlayersLayout->addWidget(row);
        this->otherPlayerInputs.append(stackEdit);
    }
}

void IcmPushFoldDialog::onOtherPlayersContinue(){
    for(QLineEdit* stackEdit : this->otherPlayerInputs){
        if(stackEdit->text().trimmed().isEmpty()){
            QMessageBox::information(this, tr("Missing data"), tr("Enter the stack for every player."));
            return;
        }
    }
    this->goToPage(4);
}

void IcmPushFoldDialog::rebuildPayoutRows(){
    QLayoutItem* child;
    while((child = this->payoutRowsLayout->takeAt(0)) != nullptr){
        if(child->widget()) child->widget()->deleteLater();
        delete child;
    }
    this->payoutInputs.clear();

    for(int i = 0; i < this->payoutCount; i++){
        QWidget* row = new QWidget();
        row->setObjectName("icmStepCard");
        QHBoxLayout* rowLayout = new QHBoxLayout(row);
        rowLayout->addWidget(new QLabel(tr("Place %1:").arg(i + 1)));
        QLineEdit* payoutEdit = new QLineEdit(row);
        payoutEdit->setValidator(new QDoubleValidator(0, 100000000, 2, payoutEdit));
        payoutEdit->setPlaceholderText(tr("$"));
        rowLayout->addWidget(payoutEdit);
        this->payoutRowsLayout->addWidget(row);
        this->payoutInputs.append(payoutEdit);
    }
}

void IcmPushFoldDialog::onBigBlindContinue(){
    if(this->bigBlindInput->text().trimmed().isEmpty()){
        QMessageBox::information(this, tr("Missing big blind"), tr("Enter the current big blind."));
        return;
    }
    this->goToPage(5);
}

void IcmPushFoldDialog::onPayoutsContinue(){
    for(QLineEdit* payoutEdit : this->payoutInputs){
        if(payoutEdit->text().trimmed().isEmpty()){
            QMessageBox::information(this, tr("Missing payouts"), tr("Enter the payout for every paid place."));
            return;
        }
    }
    this->goToPage(6);
}

void IcmPushFoldDialog::onPickHandClicked(){
    boardselector* picker = new boardselector(this->handTextEdit, QSolverJob::Mode::HOLDEM, this);
    picker->setAttribute(Qt::WA_DeleteOnClose);
    picker->setMaxCards(2);
    connect(picker, &QObject::destroyed, this, &IcmPushFoldDialog::onHandPickerClosed);
    picker->show();
}

void IcmPushFoldDialog::onHandPickerClosed(){
    QStringList cards = this->handTextEdit->toPlainText().split(",", Qt::SkipEmptyParts);
    this->handLabel->setText(cards.size() == 2 ? cards.join(" ") : tr("(not chosen)"));
}

void IcmPushFoldDialog::onBack(){
    if(this->navHistory.isEmpty()) return;
    int previous = this->navHistory.takeLast();
    this->stack->setCurrentIndex(previous);
}

void IcmPushFoldDialog::showResult(){
    QVector<float> stacks;
    stacks.append(this->heroStackInput->text().toFloat());
    stacks.append(this->villainStackInput->text().toFloat());
    for(QLineEdit* stackEdit : this->otherPlayerInputs){
        stacks.append(stackEdit->text().toFloat());
    }
    int heroIndex = 0;
    int villainIndex = 1;

    QVector<float> payouts;
    for(QLineEdit* payoutEdit : this->payoutInputs) payouts.append(payoutEdit->text().toFloat());

    float bigBlind = this->bigBlindInput->text().toFloat();
    float villainStackBB = bigBlind > 0.0f ? stacks[villainIndex] / bigBlind : 20.0f;

    QString villainRange = standardShoveRange(this->scenario, villainStackBB);

    QStringList handCards = this->handTextEdit->toPlainText().split(",", Qt::SkipEmptyParts);
    QVector<QString> heroCards = { handCards[0], handCards[1] };

    float equity = handVsRangeEquity(heroCards, villainRange, this->compairer);
    IcmCallVsFold comparison = compareCallVsFold(stacks, payouts, heroIndex, villainIndex, equity);

    // Colored via inline HTML (not setStyleSheet) so the banner keeps the
    // background/border it already gets from the #icmResultBanner QSS rule
    // in every theme -- a widget-level setStyleSheet call would override
    // that rule's background/border for just this widget.
    bool shouldCall = comparison.evCall >= comparison.evFold;
    QString verdictColor = shouldCall ? "#22c55e" : "#ff5c5c";
    QString verdictText = shouldCall ? tr("✓ Call") : tr("✕ Fold");
    this->resultLabel->setText(QString("<span style=\"color:%1; font-size:22px; font-weight:800;\">%2</span>")
        .arg(verdictColor, verdictText));

    styleCardChip(this->handCard1Chip, heroCards[0]);
    styleCardChip(this->handCard2Chip, heroCards[1]);

    QString winBorder = "border:2px solid #d4af37;";
    QString loseBorder = "border:1px solid #2a4a38;";
    this->evCallBox->setText(tr("Your value if you call\n$%1").arg(QString::number(comparison.evCall, 'f', 0)));
    this->evCallBox->setStyleSheet(QString("padding:10px; border-radius:8px; font-weight:700; %1")
        .arg(shouldCall ? winBorder : loseBorder));
    this->evFoldBox->setText(tr("Your value if you fold\n$%1").arg(QString::number(comparison.evFold, 'f', 0)));
    this->evFoldBox->setStyleSheet(QString("padding:10px; border-radius:8px; font-weight:700; %1")
        .arg(shouldCall ? loseBorder : winBorder));

    this->goToPage(7);
}
