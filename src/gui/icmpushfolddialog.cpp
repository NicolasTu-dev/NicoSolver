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

    // ---- Page 1: players/stacks + blinds/payouts (merged into one step) ----
    QWidget* stacksPage = new QWidget(this);
    QVBoxLayout* stacksPageLayout = new QVBoxLayout(stacksPage);
    stacksPageLayout->addWidget(new QLabel(tr("<b>How many players are left at the table?</b>"), stacksPage));
    QSpinBox* playerCountSpin = new QSpinBox(stacksPage);
    playerCountSpin->setRange(2, 9);
    playerCountSpin->setValue(6);
    stacksPageLayout->addWidget(playerCountSpin);

    QScrollArea* stackScroll = new QScrollArea(stacksPage);
    stackScroll->setWidgetResizable(true);
    QWidget* stackRowsWidget = new QWidget(stackScroll);
    this->stackRowsLayout = new QVBoxLayout(stackRowsWidget);
    stackScroll->setWidget(stackRowsWidget);
    stacksPageLayout->addWidget(stackScroll);

    connect(playerCountSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &IcmPushFoldDialog::onPlayerCountChanged);

    QFormLayout* blindForm = new QFormLayout();
    this->bigBlindInput = new QLineEdit(stacksPage);
    this->bigBlindInput->setValidator(new QDoubleValidator(0.01, 1000000, 2, this->bigBlindInput));
    blindForm->addRow(tr("Current big blind:"), this->bigBlindInput);
    stacksPageLayout->addLayout(blindForm);

    stacksPageLayout->addWidget(new QLabel(tr("<b>How many places get paid?</b>"), stacksPage));
    QSpinBox* payoutCountSpin = new QSpinBox(stacksPage);
    payoutCountSpin->setRange(1, 9);
    payoutCountSpin->setValue(3);
    stacksPageLayout->addWidget(payoutCountSpin);

    QScrollArea* payoutScroll = new QScrollArea(stacksPage);
    payoutScroll->setWidgetResizable(true);
    QWidget* payoutRowsWidget = new QWidget(payoutScroll);
    this->payoutRowsLayout = new QVBoxLayout(payoutRowsWidget);
    payoutScroll->setWidget(payoutRowsWidget);
    stacksPageLayout->addWidget(payoutScroll);

    connect(payoutCountSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &IcmPushFoldDialog::onPayoutCountChanged);

    QPushButton* stacksBack = new QPushButton(tr("← Back"), stacksPage);
    QPushButton* stacksNext = new QPushButton(tr("Next →"), stacksPage);
    QHBoxLayout* stacksNav = new QHBoxLayout();
    stacksNav->addWidget(stacksBack);
    stacksNav->addWidget(stacksNext);
    stacksPageLayout->addLayout(stacksNav);
    connect(stacksBack, &QPushButton::clicked, this, &IcmPushFoldDialog::onBack);
    connect(stacksNext, &QPushButton::clicked, this, &IcmPushFoldDialog::onStacksAndPayoutsContinue);
    this->stack->addWidget(stacksPage);
    this->onPlayerCountChanged(playerCountSpin->value());
    this->onPayoutCountChanged(payoutCountSpin->value());

    // ---- Page 2: hand ----
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

    // ---- Page 3: result ----
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
    connect(resultBack, &QPushButton::clicked, this, &IcmPushFoldDialog::onBack);
    resultPageLayout->addWidget(resultBack);
    this->stack->addWidget(resultPage);
}

void IcmPushFoldDialog::onScenarioContinue(){
    this->stack->setCurrentIndex(1);
}

void IcmPushFoldDialog::rebuildStackRows(int count){
    QLayoutItem* child;
    while((child = this->stackRowsLayout->takeAt(0)) != nullptr){
        if(child->widget()) child->widget()->deleteLater();
        delete child;
    }
    this->stackInputs.clear();
    this->roleInputs.clear();

    for(int i = 0; i < count; i++){
        QWidget* row = new QWidget();
        QHBoxLayout* rowLayout = new QHBoxLayout(row);
        rowLayout->addWidget(new QLabel(tr("Player %1:").arg(i + 1)));
        QLineEdit* stackEdit = new QLineEdit(row);
        stackEdit->setValidator(new QDoubleValidator(0, 100000000, 0, stackEdit));
        stackEdit->setPlaceholderText(tr("chips"));
        rowLayout->addWidget(stackEdit);
        QComboBox* roleCombo = new QComboBox(row);
        roleCombo->addItem("");
        roleCombo->addItem(tr("You"));
        roleCombo->addItem(tr("Opponent"));
        // Defaults save the common case a click: row 1 is usually "you"
        // (the player using the app) and row 2 is whoever you're comparing
        // against -- change it if that's not your situation.
        if(i == 0) roleCombo->setCurrentText(tr("You"));
        else if(i == 1) roleCombo->setCurrentText(tr("Opponent"));
        rowLayout->addWidget(roleCombo);
        this->stackRowsLayout->addWidget(row);
        this->stackInputs.append(stackEdit);
        this->roleInputs.append(roleCombo);
    }
}

void IcmPushFoldDialog::onPlayerCountChanged(int count){
    this->rebuildStackRows(count);
}

void IcmPushFoldDialog::rebuildPayoutRows(int count){
    QLayoutItem* child;
    while((child = this->payoutRowsLayout->takeAt(0)) != nullptr){
        if(child->widget()) child->widget()->deleteLater();
        delete child;
    }
    this->payoutInputs.clear();

    for(int i = 0; i < count; i++){
        QWidget* row = new QWidget();
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

void IcmPushFoldDialog::onPayoutCountChanged(int count){
    this->rebuildPayoutRows(count);
}

void IcmPushFoldDialog::onStacksAndPayoutsContinue(){
    int heroCount = 0, villainCount = 0;
    for(int i = 0; i < this->stackInputs.size(); i++){
        if(this->stackInputs[i]->text().trimmed().isEmpty()){
            QMessageBox::information(this, tr("Missing data"), tr("Enter the stack for every player."));
            return;
        }
        if(this->roleInputs[i]->currentText() == tr("You")) heroCount++;
        if(this->roleInputs[i]->currentText() == tr("Opponent")) villainCount++;
    }
    if(heroCount != 1 || villainCount != 1){
        QMessageBox::information(this, tr("Mark yourself and your opponent"),
            tr("Mark exactly one seat as 'You' and exactly one as 'Opponent'."));
        return;
    }
    if(this->bigBlindInput->text().trimmed().isEmpty()){
        QMessageBox::information(this, tr("Missing big blind"), tr("Enter the current big blind."));
        return;
    }
    for(QLineEdit* payoutEdit : this->payoutInputs){
        if(payoutEdit->text().trimmed().isEmpty()){
            QMessageBox::information(this, tr("Missing payouts"), tr("Enter the payout for every paid place."));
            return;
        }
    }
    this->stack->setCurrentIndex(2);
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
    int current = this->stack->currentIndex();
    if(current > 0) this->stack->setCurrentIndex(current - 1);
}

void IcmPushFoldDialog::showResult(){
    QVector<float> stacks;
    int heroIndex = -1, villainIndex = -1;
    for(int i = 0; i < this->stackInputs.size(); i++){
        stacks.append(this->stackInputs[i]->text().toFloat());
        if(this->roleInputs[i]->currentText() == tr("You")) heroIndex = stacks.size() - 1;
        if(this->roleInputs[i]->currentText() == tr("Opponent")) villainIndex = stacks.size() - 1;
    }

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

    this->stack->setCurrentIndex(3);
}
