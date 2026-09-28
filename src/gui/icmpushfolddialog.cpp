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
    scenarioLayout->addWidget(new QLabel(tr("<b>¿Qué pasó?</b>"), scenarioPage));
    QRadioButton* openShoveRadio = new QRadioButton(tr("Me tiraron un all-in de entrada"), scenarioPage);
    QRadioButton* reshoveRadio = new QRadioButton(tr("Abrí y me re-shovearon"), scenarioPage);
    openShoveRadio->setChecked(true);
    QButtonGroup* scenarioGroup = new QButtonGroup(scenarioPage);
    scenarioGroup->addButton(openShoveRadio);
    scenarioGroup->addButton(reshoveRadio);
    scenarioLayout->addWidget(openShoveRadio);
    scenarioLayout->addWidget(reshoveRadio);
    scenarioLayout->addStretch();
    QPushButton* scenarioNext = new QPushButton(tr("Siguiente →"), scenarioPage);
    scenarioLayout->addWidget(scenarioNext);
    connect(scenarioNext, &QPushButton::clicked, this, [this, openShoveRadio](){
        this->scenario = openShoveRadio->isChecked()
            ? PushFoldScenario::OpenShove : PushFoldScenario::ReshoveOverOpen;
        this->onScenarioContinue();
    });
    this->stack->addWidget(scenarioPage);

    // ---- Page 1: players/stacks ----
    QWidget* stacksPage = new QWidget(this);
    QVBoxLayout* stacksPageLayout = new QVBoxLayout(stacksPage);
    stacksPageLayout->addWidget(new QLabel(tr("<b>¿Cuántos jugadores quedan en la mesa?</b>"), stacksPage));
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

    QPushButton* stacksBack = new QPushButton(tr("← Atrás"), stacksPage);
    QPushButton* stacksNext = new QPushButton(tr("Siguiente →"), stacksPage);
    QHBoxLayout* stacksNav = new QHBoxLayout();
    stacksNav->addWidget(stacksBack);
    stacksNav->addWidget(stacksNext);
    stacksPageLayout->addLayout(stacksNav);
    connect(stacksBack, &QPushButton::clicked, this, &IcmPushFoldDialog::onBack);
    connect(stacksNext, &QPushButton::clicked, this, &IcmPushFoldDialog::onStacksContinue);
    this->stack->addWidget(stacksPage);
    this->onPlayerCountChanged(playerCountSpin->value());

    // ---- Page 2: blinds/payouts ----
    QWidget* payoutsPage = new QWidget(this);
    QVBoxLayout* payoutsPageLayout = new QVBoxLayout(payoutsPage);
    QFormLayout* blindForm = new QFormLayout();
    this->bigBlindInput = new QLineEdit(payoutsPage);
    this->bigBlindInput->setValidator(new QDoubleValidator(0.01, 1000000, 2, this->bigBlindInput));
    blindForm->addRow(tr("Big blind actual:"), this->bigBlindInput);
    payoutsPageLayout->addLayout(blindForm);

    payoutsPageLayout->addWidget(new QLabel(tr("<b>¿Cuántos lugares pagan?</b>"), payoutsPage));
    QSpinBox* payoutCountSpin = new QSpinBox(payoutsPage);
    payoutCountSpin->setRange(1, 9);
    payoutCountSpin->setValue(3);
    payoutsPageLayout->addWidget(payoutCountSpin);

    QScrollArea* payoutScroll = new QScrollArea(payoutsPage);
    payoutScroll->setWidgetResizable(true);
    QWidget* payoutRowsWidget = new QWidget(payoutScroll);
    this->payoutRowsLayout = new QVBoxLayout(payoutRowsWidget);
    payoutScroll->setWidget(payoutRowsWidget);
    payoutsPageLayout->addWidget(payoutScroll);

    connect(payoutCountSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &IcmPushFoldDialog::onPayoutCountChanged);

    QPushButton* payoutsBack = new QPushButton(tr("← Atrás"), payoutsPage);
    QPushButton* payoutsNext = new QPushButton(tr("Siguiente →"), payoutsPage);
    QHBoxLayout* payoutsNav = new QHBoxLayout();
    payoutsNav->addWidget(payoutsBack);
    payoutsNav->addWidget(payoutsNext);
    payoutsPageLayout->addLayout(payoutsNav);
    connect(payoutsBack, &QPushButton::clicked, this, &IcmPushFoldDialog::onBack);
    connect(payoutsNext, &QPushButton::clicked, this, &IcmPushFoldDialog::onPayoutsContinue);
    this->stack->addWidget(payoutsPage);
    this->onPayoutCountChanged(payoutCountSpin->value());

    // ---- Page 3: hand ----
    QWidget* handPage = new QWidget(this);
    QVBoxLayout* handPageLayout = new QVBoxLayout(handPage);
    handPageLayout->addWidget(new QLabel(tr("<b>Tu mano</b>"), handPage));
    this->handTextEdit = new QTextEdit(handPage);
    this->handTextEdit->setVisible(false); // used only as boardselector's storage target
    this->handLabel = new QLabel(tr("(sin elegir)"), handPage);
    handPageLayout->addWidget(this->handLabel);
    QPushButton* pickHandButton = new QPushButton(tr("Elegir tu mano"), handPage);
    handPageLayout->addWidget(pickHandButton);
    connect(pickHandButton, &QPushButton::clicked, this, &IcmPushFoldDialog::onPickHandClicked);
    handPageLayout->addStretch();

    QPushButton* handBack = new QPushButton(tr("← Atrás"), handPage);
    QPushButton* handNext = new QPushButton(tr("Ver resultado →"), handPage);
    QHBoxLayout* handNav = new QHBoxLayout();
    handNav->addWidget(handBack);
    handNav->addWidget(handNext);
    handPageLayout->addLayout(handNav);
    connect(handBack, &QPushButton::clicked, this, &IcmPushFoldDialog::onBack);
    connect(handNext, &QPushButton::clicked, this, [this](){
        QStringList cards = this->handTextEdit->toPlainText().split(",", Qt::SkipEmptyParts);
        if(cards.size() != 2){
            QMessageBox::information(this, tr("Elegí tu mano"), tr("Tenés que elegir 2 cartas antes de continuar."));
            return;
        }
        this->showResult();
    });
    this->stack->addWidget(handPage);

    // ---- Page 4: result ----
    QWidget* resultPage = new QWidget(this);
    QVBoxLayout* resultPageLayout = new QVBoxLayout(resultPage);
    this->resultLabel = new QLabel(resultPage);
    this->resultLabel->setObjectName("icmResultBanner");
    this->resultLabel->setWordWrap(true);
    resultPageLayout->addWidget(this->resultLabel);

    this->resultRangeView = new QTableView(resultPage);
    this->resultRangeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->resultRangeView->setSelectionMode(QAbstractItemView::NoSelection);
    this->resultRangeView->verticalHeader()->setMinimumSectionSize(1);
    this->resultRangeView->horizontalHeader()->setMinimumSectionSize(1);
    resultPageLayout->addWidget(this->resultRangeView);

    this->rangeMetaLabel = new QLabel(resultPage);
    this->rangeMetaLabel->setWordWrap(true);
    this->rangeMetaLabel->setStyleSheet("color:#8fb39f; font-size:12px;");
    resultPageLayout->addWidget(this->rangeMetaLabel);

    QPushButton* resultBack = new QPushButton(tr("← Atrás"), resultPage);
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
        rowLayout->addWidget(new QLabel(tr("Jugador %1:").arg(i + 1)));
        QLineEdit* stackEdit = new QLineEdit(row);
        stackEdit->setValidator(new QDoubleValidator(0, 100000000, 0, stackEdit));
        stackEdit->setPlaceholderText(tr("fichas"));
        rowLayout->addWidget(stackEdit);
        QComboBox* roleCombo = new QComboBox(row);
        roleCombo->addItem("");
        roleCombo->addItem(tr("Vos"));
        roleCombo->addItem(tr("Rival"));
        rowLayout->addWidget(roleCombo);
        this->stackRowsLayout->addWidget(row);
        this->stackInputs.append(stackEdit);
        this->roleInputs.append(roleCombo);
    }
}

void IcmPushFoldDialog::onPlayerCountChanged(int count){
    this->rebuildStackRows(count);
}

void IcmPushFoldDialog::onStacksContinue(){
    int heroCount = 0, villainCount = 0;
    for(int i = 0; i < this->stackInputs.size(); i++){
        if(this->stackInputs[i]->text().trimmed().isEmpty()){
            QMessageBox::information(this, tr("Faltan datos"), tr("Cargá el stack de cada jugador."));
            return;
        }
        if(this->roleInputs[i]->currentText() == tr("Vos")) heroCount++;
        if(this->roleInputs[i]->currentText() == tr("Rival")) villainCount++;
    }
    if(heroCount != 1 || villainCount != 1){
        QMessageBox::information(this, tr("Marcá vos y el rival"),
            tr("Marcá exactamente un asiento como 'Vos' y exactamente uno como 'Rival'."));
        return;
    }
    this->stack->setCurrentIndex(2);
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
        rowLayout->addWidget(new QLabel(tr("Puesto %1:").arg(i + 1)));
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

void IcmPushFoldDialog::onPayoutsContinue(){
    if(this->bigBlindInput->text().trimmed().isEmpty()){
        QMessageBox::information(this, tr("Falta el big blind"), tr("Cargá el big blind actual."));
        return;
    }
    for(QLineEdit* payoutEdit : this->payoutInputs){
        if(payoutEdit->text().trimmed().isEmpty()){
            QMessageBox::information(this, tr("Faltan premios"), tr("Cargá el pago de cada puesto."));
            return;
        }
    }
    this->stack->setCurrentIndex(3);
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
    this->handLabel->setText(cards.size() == 2 ? cards.join(" ") : tr("(sin elegir)"));
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
        if(this->roleInputs[i]->currentText() == tr("Vos")) heroIndex = stacks.size() - 1;
        if(this->roleInputs[i]->currentText() == tr("Rival")) villainIndex = stacks.size() - 1;
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

    float diff = comparison.evCall - comparison.evFold;
    QString verdict = diff >= 0.0f
        ? tr("✓ Pagar +$%1 vs. foldear").arg(QString::number(diff, 'f', 0))
        : tr("✕ Foldear (%1$ vs. pagar)").arg(QString::number(diff, 'f', 0));
    this->resultLabel->setText(verdict);

    this->rangeMetaLabel->setText(
        tr("Rango asumido del rival (%1, %2bb efectivas) · Tu equity: %3%")
            .arg(this->scenario == PushFoldScenario::OpenShove ? tr("all-in de entrada") : tr("re-shove sobre tu open"))
            .arg(QString::number(villainStackBB, 'f', 1))
            .arg(QString::number(equity * 100.0f, 'f', 1))
    );

    // Compute the call/fold decision for all 169 hand types (one
    // representative 2-card combo per type -- not all suited/offsuit
    // combos -- fast enough for a visual grid and accurate enough since
    // the type-level equity barely varies combo to combo).
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QStringList ranks = QString("A,K,Q,J,T,9,8,7,6,5,4,3,2").split(",");
    QStringList callingHands;
    for(int row = 0; row < ranks.size(); row++){
        for(int col = 0; col < ranks.size(); col++){
            QString rank1 = ranks[row];
            QString rank2 = ranks[col];
            QVector<QString> combo;
            QString handToken;
            if(row == col){
                combo = { rank1 + "h", rank1 + "d" };
                handToken = rank1 + rank1;
            }else if(row < col){
                combo = { rank1 + "h", rank2 + "h" };
                handToken = rank1 + rank2 + "s";
            }else{
                combo = { rank1 + "h", rank2 + "d" };
                handToken = rank2 + rank1 + "o";
            }
            float handEquity = handVsRangeEquity(combo, villainRange, this->compairer);
            IcmCallVsFold handComparison = compareCallVsFold(stacks, payouts, heroIndex, villainIndex, handEquity);
            if(handComparison.evCall >= handComparison.evFold){
                callingHands << handToken;
            }
        }
    }
    QApplication::restoreOverrideCursor();

    if(this->resultRangeModel != nullptr){
        this->resultRangeView->setModel(nullptr);
        this->resultRangeView->setItemDelegate(nullptr);
        delete this->resultRangeModel;
        delete this->resultRangeDelegate;
    }
    this->resultRangeModel = new RangeSelectorTableModel(ranks, callingHands.join(","), this, true);
    this->resultRangeDelegate = new RangeSelectorTableDelegate(ranks, this->resultRangeModel, this);
    this->resultRangeView->setModel(this->resultRangeModel);
    this->resultRangeView->setItemDelegate(this->resultRangeDelegate);

    this->stack->setCurrentIndex(4);
}
