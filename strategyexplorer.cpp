#include "strategyexplorer.h"
#include "ui_strategyexplorer.h"
#include "qstandarditemmodel.h"
#include <QBrush>
#include <QApplication>
#include <QComboBox>
#include <QColor>
#include <QToolTip>
#include <QCursor>
#include <algorithm>
#include <tuple>
#include <QFileDialog>
#include <QPixmap>
#include <QDateTime>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include "include/Card.h"

StrategyExplorer::StrategyExplorer(QWidget *parent,QSolverJob * qSolverJob) :
    QDialog(parent),
    ui(new Ui::StrategyExplorer)
{
    this->qSolverJob = qSolverJob;
    this->detailWindowSetting = DetailWindowSetting();
    ui->setupUi(this);

    this->ui->gameTreeView->setToolTip(tr("The hand's decision tree. Each level is a street (flop/turn/river) and each node is a point where a player acts. Tap a node to see the solver's strategy at that exact point."));
    this->ui->turnCardBox->setToolTip(tr("Choose which card came on the turn to see the strategy for that specific runout."));
    this->ui->riverCardBox->setToolTip(tr("Choose which card came on the river to see the strategy for that specific runout."));
    this->ui->strategyTableView->setToolTip(tr("Recommended strategy for every possible hand your opponent could have at the selected node. Blue = fold, green = call, red = bet/raise. The size of each color inside the cell is how often that action is chosen."));
    this->ui->roughStrategyView->setToolTip(tr("Simplified summary: how often, overall, the opponent folds, calls, or bets/raises with their whole range at this node."));
    this->ui->boardLabel->setToolTip(tr("The community cards on the table at this point in the hand."));
    this->ui->nodeDisplayLabel->setToolTip(tr("What action led to this node (e.g. \"OOP bet 50%\") and whose turn it is to act."));
    this->ui->advancedModeCheck->setToolTip(tr("If checked, hovering over a cell also shows the EV (expected value in chips) of each action, not just the frequency %."));
    this->ui->ipRangeButtom->setToolTip(tr("View the full range of cards the IP player could have at this node."));
    this->ui->oopRangeButtom->setToolTip(tr("View the full range of cards the OOP player could have at this node."));
    this->ui->strategyModeButtom->setToolTip(tr("Show only the strategy (how often each action is chosen) in the grid."));
    this->ui->evModeButtom->setToolTip(tr("Show the strategy together with the EV (expected value) of each action."));
    this->ui->evOnlyModeButtom->setToolTip(tr("Show only the EV (expected value) of each hand, without the strategy detail."));
    this->ui->detailView->setToolTip(tr("Combo-by-combo detail of the hand selected in the strategy grid."));
    /*
    QStandardItemModel* model = new QStandardItemModel();
    for (int row = 0; row < 4; ++row) {
         QStandardItem *item = new QStandardItem(QString("%1").arg(row) );
         model->appendRow( item );
    }
    this->ui->gameTreeView->setModel(model);
    */
    // Initial Game Tree preview panel
    this->ui->gameTreeView->setTreeData(qSolverJob);
    connect(
                this->ui->gameTreeView,
                SIGNAL(expanded(const QModelIndex&)),
                this,
                SLOT(item_expanded(const QModelIndex&))
                );
    connect(
                this->ui->gameTreeView,
                SIGNAL(clicked(const QModelIndex&)),
                this,
                SLOT(item_clicked(const QModelIndex&))
                );

    // Initize strategy(rough) table
    this->tableStrategyModel = new TableStrategyModel(this->qSolverJob,this);
    this->ui->strategyTableView->setModel(this->tableStrategyModel);
    this->delegate_strategy = new StrategyItemDelegate(this->qSolverJob,&(this->detailWindowSetting),this);
    this->ui->strategyTableView->setItemDelegate(this->delegate_strategy);

    Deck* deck = this->qSolverJob->get_solver()->get_deck();
    int index = 0;
    QString board_qstring = QString::fromStdString(this->qSolverJob->board);
    for(Card one_card: deck->getCards()){
        if(board_qstring.contains(QString::fromStdString(one_card.toString())))continue;
        QString card_str_formatted = QString::fromStdString(one_card.toFormattedString());
        this->ui->turnCardBox->addItem(card_str_formatted);
        this->ui->riverCardBox->addItem(card_str_formatted);

        if(card_str_formatted.contains(QString::fromLocal8Bit("♦️")) ||
                card_str_formatted.contains(QString::fromLocal8Bit("♥️️"))){
            this->ui->turnCardBox->setItemData(0, QBrush(Qt::red),Qt::ForegroundRole);
            this->ui->riverCardBox->setItemData(0, QBrush(Qt::red),Qt::ForegroundRole);
        }else{
            this->ui->turnCardBox->setItemData(0, QBrush(Qt::black),Qt::ForegroundRole);
            this->ui->riverCardBox->setItemData(0, QBrush(Qt::black),Qt::ForegroundRole);
        }

        this->cards.push_back(one_card);
        index += 1;
    }
    if(this->qSolverJob->get_solver()->getGameTree()->getRoot()->getRound() == GameTreeNode::GameRound::FLOP){
        this->tableStrategyModel->setTrunCard(this->cards[0]);
        this->tableStrategyModel->setRiverCard(this->cards[1]);
        this->ui->riverCardBox->setCurrentIndex(1);
    }
    else if(this->qSolverJob->get_solver()->getGameTree()->getRoot()->getRound() == GameTreeNode::GameRound::TURN){
        this->tableStrategyModel->setRiverCard(this->cards[0]);
        this->ui->turnCardBox->clear();
    }
    else if(this->qSolverJob->get_solver()->getGameTree()->getRoot()->getRound() == GameTreeNode::GameRound::RIVER){
        this->ui->turnCardBox->clear();
        this->ui->riverCardBox->clear();
    }

    // Initize timer for strategy auto update
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update_second()));
    timer->start(1000);

    // On mouse event of strategy table
    connect(this->ui->strategyTableView,SIGNAL(itemMouseChange(int,int)),this,SLOT(onMouseMoveEvent(int,int)));

    // Initize Detail Viewer window
    this->detailViewerModel = new DetailViewerModel(this->tableStrategyModel,this);
    this->ui->detailView->setModel(this->detailViewerModel);
    this->detailItemItemDelegate = new DetailItemDelegate(&(this->detailWindowSetting),this);
    this->ui->detailView->setItemDelegate(this->detailItemItemDelegate);

    // Initize Rough Strategy Viewer
    this->roughStrategyViewerModel = new RoughStrategyViewerModel(this->tableStrategyModel,this);
    this->ui->roughStrategyView->setModel(this->roughStrategyViewerModel);
    this->roughStrategyItemDelegate = new RoughStrategyItemDelegate(&(this->detailWindowSetting),this);
    this->ui->roughStrategyView->setItemDelegate(this->roughStrategyItemDelegate);

    // Start in the simpler view: just the hand banner, legend and main grid.
    // The tree, rough-strategy panel, and mode buttons are one click away
    // behind "Ver árbol de decisiones y opciones avanzadas" for people who
    // want to dig deeper.
    this->setAdvancedViewVisible(false);

    // Soft depth: make the recommended-play banner and the summary panel
    // look "lifted" off the background, like a card on felt.
    auto bannerShadow = new QGraphicsDropShadowEffect(this);
    bannerShadow->setBlurRadius(24);
    bannerShadow->setOffset(0, 3);
    bannerShadow->setColor(QColor(0, 0, 0, 140));
    this->ui->handBannerLabel->setGraphicsEffect(bannerShadow);

    auto summaryShadow = new QGraphicsDropShadowEffect(this);
    summaryShadow->setBlurRadius(24);
    summaryShadow->setOffset(0, 3);
    summaryShadow->setColor(QColor(0, 0, 0, 140));
    this->ui->groupBox_2->setGraphicsEffect(summaryShadow);
}

void StrategyExplorer::stopAutoUpdate(){
    if(this->timer != NULL) this->timer->stop();
}

void StrategyExplorer::setRunoutPickerVisible(bool visible){
    this->ui->label->setVisible(visible);
    this->ui->turnCardBox->setVisible(visible);
    this->ui->label_2->setVisible(visible);
    this->ui->riverCardBox->setVisible(visible);
}

StrategyExplorer::~StrategyExplorer()
{
    delete ui;
    delete this->delegate_strategy;
    delete this->tableStrategyModel;
    delete this->detailViewerModel;
    delete this->roughStrategyViewerModel;
    delete this->timer;
}

void StrategyExplorer::item_expanded(const QModelIndex& index){
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    int num_child = item->childCount();
    for (int i = 0;i < num_child;i ++){
        TreeItem* one_child = item->child(i);
        if(one_child->childCount() != 0)continue;
        this->ui->gameTreeView->tree_model->reGenerateTreeItem(one_child->m_treedata.lock()->getRound(),one_child);
    }
}

void StrategyExplorer::process_board(TreeItem* treeitem){
    vector<string> board_str_arr = string_split(this->qSolverJob->board,',');
    vector<Card> cards;
    for(string one_board_str:board_str_arr){
        cards.push_back(Card(one_board_str));
    }
    if(treeitem != NULL){
        if(treeitem->m_treedata.lock()->getRound() == GameTreeNode::GameRound::TURN && !this->tableStrategyModel->getTrunCard().empty()){
            cards.push_back(Card(this->tableStrategyModel->getTrunCard()));
        }
        else if(treeitem->m_treedata.lock()->getRound() == GameTreeNode::GameRound::RIVER){
            if(!this->tableStrategyModel->getTrunCard().empty())
                cards.push_back(Card(this->tableStrategyModel->getTrunCard()));
            if(!this->tableStrategyModel->getRiverCard().empty())
                cards.push_back(Card(this->tableStrategyModel->getRiverCard()));
        }
    }
    this->ui->boardLabel->setText(QString("<b>%1: </b>").arg(tr("board")) + Card::boardCards2html(cards));
}

void StrategyExplorer::process_treeclick(TreeItem* treeitem){
    shared_ptr<GameTreeNode> treenode = treeitem->m_treedata.lock();
    if(treenode->getType() == GameTreeNode::GameTreeNodeType::ACTION){
        shared_ptr<ActionNode> actionnode = static_pointer_cast<ActionNode>(treenode);
        QString action_str = QString("<b>%1 %2</b>").arg(actionnode->getPlayer() == 0?tr("IP"):tr("OOP"),tr(" decision node"));
        this->ui->nodeDisplayLabel->setText(action_str);
    }
    else if(treenode->getType() == GameTreeNode::GameTreeNodeType::CHANCE){
        QString chance_str = tr("<b>Chance node</b>");
        this->ui->nodeDisplayLabel->setText(chance_str);
    }
    else if(treenode->getType() == GameTreeNode::GameTreeNodeType::TERMINAL){
        QString terminal_str = tr("<b>Terminal node</b>");
        this->ui->nodeDisplayLabel->setText(terminal_str);
    }
    else if(treenode->getType() == GameTreeNode::GameTreeNodeType::SHOWDOWN){
        QString showdown_str = tr("<b>Showdown node</b>");
        this->ui->nodeDisplayLabel->setText(showdown_str);
    }
}

void StrategyExplorer::item_clicked(const QModelIndex& index){
    try{
        TreeItem * treeNode = static_cast<TreeItem*>(index.internalPointer());
        this->process_treeclick(treeNode);
        this->process_board(treeNode);
        this->tableStrategyModel->setGameTreeNode(treeNode);
        this->tableStrategyModel->updateStrategyData();
        this->ui->strategyTableView->viewport()->update();
        this->roughStrategyViewerModel->onchanged();
        this->ui->roughStrategyView->triger_resize();
        this->ui->roughStrategyView->viewport()->update();
        this->updateRangeSummaryLabel();
    }
    catch (const runtime_error& error)
    {
        qDebug().noquote() << tr("Encountering error:");//.toStdString() << endl;
        qDebug().noquote() << error.what() << "\n";
    }
}

void StrategyExplorer::selection_changed(const QItemSelection &selected,
                                         const QItemSelection &deselected){
}

void StrategyExplorer::on_turnCardBox_currentIndexChanged(int index)
{
    if(this->cards.size() > 0 && index < this->cards.size()){
        this->tableStrategyModel->setTrunCard(this->cards[index]);
        this->tableStrategyModel->updateStrategyData();
        // TODO this somehow cause bugs, crashes, why?
        //this->roughStrategyViewerModel->onchanged();
        //this->ui->roughStrategyView->viewport()->update();
        this->process_board(this->tableStrategyModel->treeItem);
    }
    this->ui->strategyTableView->viewport()->update();
    this->ui->detailView->viewport()->update();
}

void StrategyExplorer::on_riverCardBox_currentIndexChanged(int index)
{
    if(this->cards.size() > 0  && index < this->cards.size()){
        this->tableStrategyModel->setRiverCard(this->cards[index]);
        this->tableStrategyModel->updateStrategyData();
        //this->roughStrategyViewerModel->onchanged();
        //this->ui->roughStrategyView->viewport()->update();
        this->process_board(this->tableStrategyModel->treeItem);
    }
    this->ui->strategyTableView->viewport()->update();
    this->ui->detailView->viewport()->update();
}

void StrategyExplorer::update_second(){
    if(this->cards.size() > 0){
        this->tableStrategyModel->updateStrategyData();
        this->roughStrategyViewerModel->onchanged();
        this->ui->roughStrategyView->viewport()->update();
        this->process_board(this->tableStrategyModel->treeItem);
    }
    this->ui->strategyTableView->viewport()->update();
    this->ui->detailView->viewport()->update();
}


void StrategyExplorer::onMouseMoveEvent(int i,int j){
    this->detailWindowSetting.grid_i = i;
    this->detailWindowSetting.grid_j = j;
    this->ui->detailView->viewport()->update();
    this->ui->strategyTableView->viewport()->update();

    // Build a hover tooltip summarizing combo / action% / EV for the hovered cell,
    // reusing the same data access pattern as DetailItemDelegate::paint_strategy/paint_evs.
    QString tooltip_text;
    if(this->tableStrategyModel->treeItem != NULL &&
            this->tableStrategyModel->treeItem->m_treedata.lock()->getType() == GameTreeNode::GameTreeNode::ACTION &&
            i >= 0 && j >= 0 &&
            i < (int)this->tableStrategyModel->ui_strategy_table.size() &&
            j < (int)this->tableStrategyModel->ui_strategy_table[i].size()){
        shared_ptr<GameTreeNode> node = this->tableStrategyModel->treeItem->m_treedata.lock();
        vector<pair<int,int>>& cell_combos = this->tableStrategyModel->ui_strategy_table[i][j];
        shared_ptr<ActionNode> actionNode = dynamic_pointer_cast<ActionNode>(node);
        vector<GameActions>& gameActions = actionNode->getActions();

        if(!cell_combos.empty()){
            // Use the first combo in this cell as the representative hand for the tooltip.
            pair<int,int> combo = cell_combos[0];
            int card1 = combo.first;
            int card2 = combo.second;
            vector<float> strategy = this->tableStrategyModel->current_strategy[card1][card2];
            vector<float> evs = this->tableStrategyModel->current_evs.empty()?
                        vector<float>(gameActions.size(),-1.0f):
                        this->tableStrategyModel->current_evs[card1][card2];

            if(gameActions.size() == strategy.size()){
                QString combo_str = QString::fromStdString(this->tableStrategyModel->cardint2card[card1].toString()) +
                        QString::fromStdString(this->tableStrategyModel->cardint2card[card2].toString());
                if(cell_combos.size() > 1){
                    tooltip_text += QString("%1 (%2 combos)\n").arg(combo_str).arg(cell_combos.size());
                }else{
                    tooltip_text += QString("%1\n").arg(combo_str);
                }

                bool has_evs = gameActions.size() == evs.size();
                for(std::size_t k = 0;k < strategy.size();k ++){
                    GameActions one_action = gameActions[k];
                    float one_strategy = strategy[k] * 100;
                    QString action_name;
                    if(one_action.getAction() == GameTreeNode::PokerActions::FOLD) action_name = tr("FOLD");
                    else if(one_action.getAction() == GameTreeNode::PokerActions::CALL) action_name = tr("CALL");
                    else if(one_action.getAction() == GameTreeNode::PokerActions::CHECK) action_name = tr("CHECK");
                    else if(one_action.getAction() == GameTreeNode::PokerActions::BET) action_name = QString("%1 %2").arg(tr("BET"),QString::number(one_action.getAmount()));
                    else if(one_action.getAction() == GameTreeNode::PokerActions::RAISE) action_name = QString("%1 %2").arg(tr("RAISE"),QString::number(one_action.getAmount()));

                    // EV/equity detail is only shown in "advanced mode" — the basic tooltip
                    // (combo + action %) always shows regardless of this->advancedMode.
                    if(this->advancedMode){
                        QString ev_str;
                        if(has_evs){
                            ev_str = evs[k] != evs[k] ? tr("Can't calculate") : QString::number(evs[k],'f',1);
                        }

                        if(has_evs){
                            tooltip_text += QString("%1: %2%   %3 %4\n").arg(action_name,QString::number(one_strategy,'f',1),tr("EV"),ev_str);
                        }else{
                            tooltip_text += QString("%1: %2%\n").arg(action_name,QString::number(one_strategy,'f',1));
                        }
                    }else{
                        tooltip_text += QString("%1: %2%\n").arg(action_name,QString::number(one_strategy,'f',1));
                    }
                }
            }
        }
    }

    if(!tooltip_text.isEmpty()){
        QToolTip::showText(QCursor::pos(), tooltip_text.trimmed(), this->ui->strategyTableView);
    }else{
        QToolTip::hideText();
    }
}

void StrategyExplorer::on_strategyModeButtom_clicked()
{
    this->detailWindowSetting.mode = DetailWindowSetting::DetailWindowMode::STRATEGY;
    this->ui->strategyTableView->viewport()->update();
    this->ui->detailView->viewport()->update();
    this->roughStrategyViewerModel->onchanged();
    this->ui->roughStrategyView->viewport()->update();
}

void StrategyExplorer::on_ipRangeButtom_clicked()
{
    this->detailWindowSetting.mode = DetailWindowSetting::DetailWindowMode::RANGE_IP;
    this->ui->strategyTableView->viewport()->update();
    this->ui->detailView->viewport()->update();
    this->roughStrategyViewerModel->onchanged();
    this->ui->roughStrategyView->viewport()->update();
}

void StrategyExplorer::on_oopRangeButtom_clicked()
{
    this->detailWindowSetting.mode = DetailWindowSetting::DetailWindowMode::RANGE_OOP;
    this->ui->strategyTableView->viewport()->update();
    this->ui->detailView->viewport()->update();
    this->roughStrategyViewerModel->onchanged();
    this->ui->roughStrategyView->viewport()->update();
}

void StrategyExplorer::on_evModeButtom_clicked()
{
    this->detailWindowSetting.mode = DetailWindowSetting::DetailWindowMode::EV;
    this->ui->strategyTableView->viewport()->update();
    this->ui->detailView->viewport()->update();
    this->ui->roughStrategyView->viewport()->update();
}

void StrategyExplorer::on_evOnlyModeButtom_clicked()
{
    this->detailWindowSetting.mode = DetailWindowSetting::DetailWindowMode::EV_ONLY;
    this->ui->strategyTableView->viewport()->update();
    this->ui->detailView->viewport()->update();
    this->roughStrategyViewerModel->onchanged();
    this->ui->roughStrategyView->viewport()->update();
}

void StrategyExplorer::on_advancedModeCheck_toggled(bool checked)
{
    this->advancedMode = checked;
    this->ui->strategyTableView->viewport()->update();
    this->ui->detailView->viewport()->update();
}

void StrategyExplorer::selectRootNode(){
    QModelIndex rootIndex = this->ui->gameTreeView->model()->index(0, 0);
    if(rootIndex.isValid()){
        this->item_clicked(rootIndex);
    }
}

static QString sizeTag(double amount){
    if(amount < 45) return QObject::tr("small");
    if(amount <= 85) return QObject::tr("medium");
    if(amount <= 130) return QObject::tr("big");
    return QObject::tr("overbet");
}

static QString actionLabel(GameTreeNode::PokerActions action, double amount){
    switch(action){
        case GameTreeNode::PokerActions::FOLD: return QObject::tr("Fold");
        case GameTreeNode::PokerActions::CHECK: return QObject::tr("Check");
        case GameTreeNode::PokerActions::CALL: return QObject::tr("Call");
        case GameTreeNode::PokerActions::BET: return QObject::tr("Bet %1% of the pot (%2)").arg((int)amount).arg(sizeTag(amount));
        case GameTreeNode::PokerActions::RAISE: return QObject::tr("Raise to %1% of the pot (%2)").arg((int)amount).arg(sizeTag(amount));
        default: return QObject::tr("Other action");
    }
}

static QString actionReason(GameTreeNode::PokerActions action, bool isTopAction){
    switch(action){
        case GameTreeNode::PokerActions::FOLD:
            return QObject::tr("the hand isn't good enough to continue, continuing costs more than it can win");
        case GameTreeNode::PokerActions::CHECK:
            return QObject::tr("good enough to see the next card for free without risking more");
        case GameTreeNode::PokerActions::CALL:
            return QObject::tr("the hand is good enough to continue, but not to bet more");
        case GameTreeNode::PokerActions::BET:
        case GameTreeNode::PokerActions::RAISE:
            return isTopAction
                ? QObject::tr("builds the pot with a strong hand and pressures the opponent into a mistake")
                : QObject::tr("mixed in occasionally to stay unpredictable with this hand");
        default:
            return QString();
    }
}

void StrategyExplorer::setHighlightedHand(QString card1, QString card2){
    QStringList ranks = QString("A,K,Q,J,T,9,8,7,6,5,4,3,2").split(",");
    QChar rank1 = card1.at(0).toUpper();
    QChar rank2 = card2.at(0).toUpper();
    QChar suit1 = card1.at(1).toLower();
    QChar suit2 = card2.at(1).toLower();
    int idx1 = ranks.indexOf(QString(rank1));
    int idx2 = ranks.indexOf(QString(rank2));
    if(idx1 < 0 || idx2 < 0){
        this->ui->handBannerLabel->setVisible(false);
        return;
    }
    int i, j;
    if(idx1 == idx2){
        i = idx1; j = idx1;
    }else if(suit1 == suit2){
        i = qMin(idx1, idx2); j = qMax(idx1, idx2);
    }else{
        i = qMax(idx1, idx2); j = qMin(idx1, idx2);
    }
    if(this->tableStrategyModel->treeItem == NULL ||
       this->tableStrategyModel->treeItem->m_treedata.lock()->getType() != GameTreeNode::GameTreeNode::ACTION){
        this->ui->handBannerLabel->setVisible(false);
        return;
    }
    vector<pair<GameActions,float>> strategy = this->tableStrategyModel->get_strategy(i, j);
    if(strategy.empty()){
        this->ui->handBannerLabel->setVisible(false);
        return;
    }
    vector<float> evs = this->tableStrategyModel->get_strategies_evs(i, j);
    vector<std::tuple<GameActions,float,float>> combined;
    for(std::size_t k = 0; k < strategy.size(); k++){
        float ev = k < evs.size() ? evs[k] : 0.f;
        combined.push_back(std::make_tuple(strategy[k].first, strategy[k].second, ev));
    }
    std::sort(combined.begin(), combined.end(), [](const std::tuple<GameActions,float,float>& a, const std::tuple<GameActions,float,float>& b){
        return std::get<1>(a) > std::get<1>(b);
    });
    QString handLabel = QString("%1%2").arg(card1.at(0).toUpper()).arg(card2.at(0).toUpper());
    if(idx1 != idx2) handLabel += (suit1 == suit2) ? "s" : "o";

    QStringList parts;
    bool isTop = true;
    float topEv = combined.empty() ? 0.f : std::get<2>(combined[0]);
    for(std::tuple<GameActions,float,float> entry : combined){
        float freq = std::get<1>(entry);
        if(freq < 0.01f) continue;
        int pct = (int)(freq * 100 + 0.5f);
        GameActions action = std::get<0>(entry);
        QString piece = QString("%1 (%2%)").arg(actionLabel(action.getAction(), action.getAmount())).arg(pct);
        QString reason = actionReason(action.getAction(), isTop);
        if(!reason.isEmpty()) piece += QString(" — %1").arg(reason);
        if(!isTop){
            float evLoss = topEv - std::get<2>(entry);
            if(evLoss > 0.01f){
                piece += tr(" [costs ~%1 fewer chips than the best play]").arg(QString::number(evLoss, 'f', 1));
            }
        }
        parts << piece;
        isTop = false;
    }
    this->ui->handBannerLabel->setText(tr("With %1:<br>%2").arg(handLabel).arg(parts.join("<br>")));
    this->ui->handBannerLabel->setVisible(true);
}

std::tuple<float,float,float> StrategyExplorer::getRootActionSummary(){
    float foldPct = 0.f, callPct = 0.f, betPct = 0.f;
    if(this->tableStrategyModel->treeItem == NULL ||
       this->tableStrategyModel->treeItem->m_treedata.lock()->getType() != GameTreeNode::GameTreeNode::ACTION){
        return std::make_tuple(foldPct, callPct, betPct);
    }
    const vector<pair<GameActions,pair<float,float>>>& totalStrategy = this->tableStrategyModel->total_strategy;
    for(const pair<GameActions,pair<float,float>>& entry : totalStrategy){
        GameActions action = entry.first;
        float freq = entry.second.second;
        if(action.getAction() == GameTreeNode::PokerActions::FOLD) foldPct += freq;
        else if(action.getAction() == GameTreeNode::PokerActions::CHECK || action.getAction() == GameTreeNode::PokerActions::CALL) callPct += freq;
        else betPct += freq;
    }
    return std::make_tuple(foldPct, callPct, betPct);
}

void StrategyExplorer::updateRangeSummaryLabel(){
    if(this->tableStrategyModel->treeItem == NULL ||
       this->tableStrategyModel->treeItem->m_treedata.lock()->getType() != GameTreeNode::GameTreeNode::ACTION){
        this->ui->roughStrategyIntroLabel->setText(tr("With your opponent's whole range at this point: how often it's correct to fold (blue), call (green), or bet/raise (red)."));
        return;
    }
    const vector<pair<GameActions,pair<float,float>>>& totalStrategy = this->tableStrategyModel->total_strategy;
    if(totalStrategy.empty()){
        this->ui->roughStrategyIntroLabel->setText(tr("With your opponent's whole range at this point: how often it's correct to fold (blue), call (green), or bet/raise (red)."));
        return;
    }
    float foldPct = 0.f, callPct = 0.f, betPct = 0.f;
    for(const pair<GameActions,pair<float,float>>& entry : totalStrategy){
        GameActions action = entry.first;
        float freq = entry.second.second;
        if(action.getAction() == GameTreeNode::PokerActions::FOLD) foldPct += freq;
        else if(action.getAction() == GameTreeNode::PokerActions::CHECK || action.getAction() == GameTreeNode::PokerActions::CALL) callPct += freq;
        else betPct += freq;
    }
    this->ui->roughStrategyIntroLabel->setText(
        tr("Your opponent gets here and, with their whole range, folds %1% of the time, calls/checks %2%, and bets/raises %3%.")
            .arg((int)(foldPct * 100 + 0.5f))
            .arg((int)(callPct * 100 + 0.5f))
            .arg((int)(betPct * 100 + 0.5f))
    );
}

void StrategyExplorer::setAdvancedViewVisible(bool visible){
    this->advancedViewVisible = visible;
    this->ui->groupBox->setVisible(visible);
    this->ui->label_4->setVisible(visible);
    this->ui->ipRangeButtom->setVisible(visible);
    this->ui->oopRangeButtom->setVisible(visible);
    this->ui->label_5->setVisible(visible);
    this->ui->strategyModeButtom->setVisible(visible);
    this->ui->evModeButtom->setVisible(visible);
    this->ui->evOnlyModeButtom->setVisible(visible);
    this->ui->detailView->setVisible(visible);
    this->ui->toggleAdvancedViewButton->setText(visible ?
        tr("🔍 Hide decision tree and advanced options") :
        tr("🔍 View decision tree and advanced options"));
}

void StrategyExplorer::on_toggleAdvancedViewButton_clicked(){
    this->setAdvancedViewVisible(!this->advancedViewVisible);
}

void StrategyExplorer::on_exportImageButton_clicked(){
    QString defaultName = tr("solverix_play_%1.png").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    QString path = QFileDialog::getSaveFileName(this, tr("Export as image"), defaultName, tr("PNG image (*.png)"));
    if(path.isEmpty()) return;
    if(!path.endsWith(".png", Qt::CaseInsensitive)) path += ".png";
    QPixmap snapshot = this->grab();
    if(snapshot.save(path, "PNG")){
        QMessageBox::information(this, tr("Done"), tr("Image saved to:\n%1").arg(path));
    }else{
        QMessageBox::warning(this, tr("Error"), tr("Couldn't save the image."));
    }
}

void StrategyExplorer::on_glossaryButton_clicked(){
    QString glossary = tr(
        "<h3>Quick glossary</h3>"
        "<p><b>EV (expected value):</b> how much a play wins or loses on average, in chips, if repeated many times. A higher EV is better.</p>"
        "<p><b>Range:</b> the set of possible hands a player could have at a point in the hand, not an exact card.</p>"
        "<p><b>Blocker:</b> holding a card that makes it less likely your opponent has a certain strong hand (e.g. holding an Ace reduces the AA combos your opponent could have).</p>"
        "<p><b>Indifference:</b> a point where two plays give exactly the same expected result, which is why the solver sometimes mixes between two actions with the same hand.</p>"
        "<p><b>IP / OOP:</b> IP (in position) is the player who acts last on the street; OOP (out of position) is the one who acts first. Playing in position (IP) is an advantage.</p>"
        "<p><b>Combos:</b> the number of exact card combinations that make up a hand (e.g. AKs has 4 combos, one per suit).</p>"
        "<p><b>Frequency:</b> how often the solver chooses an action with a given hand, expressed as a percentage.</p>"
    );
    QMessageBox box(this);
    box.setWindowTitle(tr("Glossary"));
    box.setTextFormat(Qt::RichText);
    box.setText(glossary);
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}
