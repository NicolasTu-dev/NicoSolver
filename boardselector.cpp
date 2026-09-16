#include "boardselector.h"
#include "ui_boardselector.h"
#include <QToolTip>
#include <QCursor>

boardselector::boardselector(QTextEdit* boardEdit,QSolverJob::Mode mode,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::boardselector)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("BoardSelector"));
    this->boardEdit = boardEdit;
    this->mode = mode;

    QString ranks;
    if(mode == QSolverJob::Mode::HOLDEM){
        ranks = "A,K,Q,J,T,9,8,7,6,5,4,3,2";
    }else if(mode == QSolverJob::Mode::SHORTDECK){
        ranks = "A,K,Q,J,T,9,8,7,6";
    }else{
        throw runtime_error("mode not found in range selector");
    }
    this->rank_list = ranks.split(",");

    this->boardSelectorTableModel = new BoardSelectorTableModel(this->rank_list,boardEdit->toPlainText(),this);
    this->ui->boardSelectorTable->setModel(boardSelectorTableModel);
    this->boardSelectorTableDelegate = new BoardSelectorTableDelegate(this->rank_list,this->boardSelectorTableModel,this);
    this->ui->boardSelectorTable->setItemDelegate(this->boardSelectorTableDelegate);
    this->ui->boardEdit->setText(boardEdit->toPlainText());
}

boardselector::~boardselector()
{
    delete ui;
}

void boardselector::setMaxCards(int maxCards)
{
    this->maxCards = maxCards;
}

void boardselector::on_boardSelectorTable_clicked(const QModelIndex &index)
{
    int row = index.row();
    int col = index.column();
    bool isSelecting = this->boardSelectorTableModel->getBoardAt(row,col) == 0;
    int currentCount = this->boardSelectorTableModel->getBoardText().split(",", Qt::SkipEmptyParts).size();
    if(isSelecting && currentCount >= this->maxCards){
        QToolTip::showText(QCursor::pos(), this->maxCards >= 5
            ? tr("A board can only have up to 5 cards (flop + turn + river).")
            : tr("You can only add 1 card here."));
        return;
    }
    this->boardSelectorTableModel->setBoardAt(row,col,1 - this->boardSelectorTableModel->getBoardAt(row,col));
    if(this->ui->boardEdit->text() != this->boardSelectorTableModel->getBoardText()){
        this->ui->boardEdit->setText(this->boardSelectorTableModel->getBoardText());
    }
    this->ui->boardEdit->setFocus();
    this->ui->boardSelectorTable->setFocus();
}

void boardselector::on_boardEdit_textEdited(const QString &arg1)
{
}

void boardselector::on_boardEdit_textChanged(const QString &arg1)
{
    this->boardSelectorTableModel->setBoardText(arg1);
    this->ui->boardSelectorTable->update();
    this->ui->boardSelectorTable->setFocus();
    this->ui->boardEdit->setFocus();
}

void boardselector::on_confirmButton_clicked()
{
    this->boardEdit->setText(this->ui->boardEdit->text());
    this->close();
}

void boardselector::on_cancelButton_clicked()
{
    this->close();
}

void boardselector::on_clearBoardButton_clicked()
{
    this->boardSelectorTableModel->clear_board();
    this->ui->boardEdit->clear();
    this->ui->boardSelectorTable->update();
    this->ui->boardSelectorTable->setFocus();
    this->ui->boardEdit->setFocus();
}
