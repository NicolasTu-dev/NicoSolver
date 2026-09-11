#include "include/ui/boardselectortabledelegate.h"

BoardSelectorTableDelegate::BoardSelectorTableDelegate(QStringList ranks,BoardSelectorTableModel *boardSelectorTableModel,QObject *parent):WordItemDelegate(parent){
    this->rank_list = ranks;
    this->boardSelectorTableModel = boardSelectorTableModel;
}

void BoardSelectorTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const{

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    const qreal margin = 3.0;
    QRectF cellRect(option.rect.left() + margin, option.rect.top() + margin,
                     option.rect.width() - margin * 2, option.rect.height() - margin * 2);
    const qreal radius = 5.0;
    QPainterPath cardPath;
    cardPath.addRoundedRect(cellRect, radius, radius);

    QString cardStr = index.model()->data(index, Qt::DisplayRole).toString();
    QChar rankChar = cardStr.length() > 0 ? cardStr.at(0) : QChar(' ');
    QChar suitChar = cardStr.length() > 1 ? cardStr.at(1).toLower() : QChar(' ');

    QString suitSymbol = "?";
    QColor suitColor = QColor("#1a1d29");
    if(suitChar == 's'){ suitSymbol = QString::fromUtf8("\xE2\x99\xA0"); suitColor = QColor("#1a1d29"); }
    else if(suitChar == 'h'){ suitSymbol = QString::fromUtf8("\xE2\x99\xA5"); suitColor = QColor("#d1352b"); }
    else if(suitChar == 'd'){ suitSymbol = QString::fromUtf8("\xE2\x99\xA6"); suitColor = QColor("#d1352b"); }
    else if(suitChar == 'c'){ suitSymbol = QString::fromUtf8("\xE2\x99\xA3"); suitColor = QColor("#1a1d29"); }

    bool isSelected = this->boardSelectorTableModel->getBoardAt(index.row(), index.column()) > 0.0f;

    // Card face: always a light "real card" color, independent of app theme.
    painter->fillPath(cardPath, QColor("#f5f2e8"));
    painter->setPen(QPen(QColor("#c9c4b0"), 1));
    painter->drawPath(cardPath);

    QFont rankFont = painter->font();
    rankFont.setBold(true);
    rankFont.setPointSizeF(qMax(7.0, cellRect.height() * 0.22));
    painter->setFont(rankFont);
    painter->setPen(suitColor);
    QRectF rankRect(cellRect.left() + 4, cellRect.top() + 2, cellRect.width() - 8, cellRect.height() * 0.32);
    painter->drawText(rankRect, Qt::AlignLeft | Qt::AlignTop, QString(rankChar));

    QFont suitFont = painter->font();
    suitFont.setBold(false);
    suitFont.setPointSizeF(qMax(9.0, cellRect.height() * 0.4));
    painter->setFont(suitFont);
    painter->drawText(cellRect, Qt::AlignCenter, suitSymbol);

    if(isSelected){
        painter->setPen(QPen(QColor("#22c55e"), 3));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(cardPath);

        QRectF badge(cellRect.right() - 15, cellRect.top() + 2, 13, 13);
        painter->setBrush(QColor("#22c55e"));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(badge);
        painter->setPen(QPen(Qt::white, 1.6));
        painter->drawLine(QPointF(badge.left() + 2.5, badge.center().y()),
                           QPointF(badge.center().x() - 0.5, badge.bottom() - 3));
        painter->drawLine(QPointF(badge.center().x() - 0.5, badge.bottom() - 3),
                           QPointF(badge.right() - 2.5, badge.top() + 3));
    }

    painter->restore();
}
