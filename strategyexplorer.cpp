#include "strategyexplorer.h"
#include "ui_strategyexplorer.h"
#include "qstandarditemmodel.h"
#include <QBrush>
#include <QApplication>
#include <QComboBox>
#include <QColor>
#include <QToolTip>
#include <QCursor>
#include "include/Card.h"

StrategyExplorer::StrategyExplorer(QWidget *parent,QSolverJob * qSolverJob) :
    QDialog(parent),
    ui(new Ui::StrategyExplorer)
{
    this->qSolverJob = qSolverJob;
    this->detailWindowSetting = DetailWindowSetting();
    ui->setupUi(this);

    this->ui->gameTreeView->setToolTip(tr("Árbol de decisiones de la mano. Cada nivel es una calle (flop/turn/river) y cada nodo es un punto donde un jugador actúa. Tocá un nodo para ver la estrategia del solver en ese punto exacto."));
    this->ui->turnCardBox->setToolTip(tr("Elegí qué carta cayó en el turn para ver la estrategia en ese runout específico."));
    this->ui->riverCardBox->setToolTip(tr("Elegí qué carta cayó en el river para ver la estrategia en ese runout específico."));
    this->ui->strategyTableView->setToolTip(tr("Estrategia recomendada para cada mano posible del rival en el nodo seleccionado. Celeste = retirarse, verde = pagar, rojo = apostar/subir. El tamaño de cada color dentro de la celda es qué tan seguido se elige esa acción."));
    this->ui->roughStrategyView->setToolTip(tr("Resumen simplificado: qué tan seguido, en total, el rival retira, paga o apuesta/sube con todo su rango en este nodo."));
    this->ui->boardLabel->setToolTip(tr("Las cartas comunitarias de la mesa en este punto de la mano."));
    this->ui->nodeDisplayLabel->setToolTip(tr("Qué acción llevó a este nodo (por ejemplo, \"OOP bet 50%\") y de quién es el turno de actuar."));
    this->ui->advancedModeCheck->setToolTip(tr("Si lo tildás, el detalle al pasar el mouse sobre una celda también muestra el EV (ganancia esperada en fichas) de cada acción, no solo el % de frecuencia."));
    this->ui->ipRangeButtom->setToolTip(tr("Ver el rango completo de cartas que puede tener el jugador IP en este nodo."));
    this->ui->oopRangeButtom->setToolTip(tr("Ver el rango completo de cartas que puede tener el jugador OOP en este nodo."));
    this->ui->strategyModeButtom->setToolTip(tr("Mostrar solo la estrategia (qué tan seguido se elige cada acción) en la grilla."));
    this->ui->evModeButtom->setToolTip(tr("Mostrar la estrategia junto con el EV (ganancia esperada) de cada acción."));
    this->ui->evOnlyModeButtom->setToolTip(tr("Mostrar solo el EV (ganancia esperada) de cada mano, sin el detalle de la estrategia."));
    this->ui->detailView->setToolTip(tr("Detalle combo por combo de la mano seleccionada en la grilla de estrategia."));
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
