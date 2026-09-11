#include "include/ui/rangeselectortabledelegate.h"

QColor RangeSelectorTableDelegate::accentColor = QColor("#5b6ef5");
QColor RangeSelectorTableDelegate::emptyColor = QColor("#1a1d29");
QColor RangeSelectorTableDelegate::pairColor = QColor("#242838");
QColor RangeSelectorTableDelegate::borderColor = QColor("#0d0f16");

RangeSelectorTableDelegate::RangeSelectorTableDelegate(QStringList ranks,RangeSelectorTableModel *rangeSelectorTableModel,QObject *parent):WordItemDelegate(parent){
    this->rank_list = ranks;
    this->rangeSelectorTableModel = rangeSelectorTableModel;
}

void RangeSelectorTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const{

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    auto options = option;
    initStyleOption(&options, index);

    const int margin = 1;
    QRectF cellRect(option.rect.left() + margin, option.rect.top() + margin,
                     option.rect.width() - margin * 2, option.rect.height() - margin * 2);
    const qreal radius = 3.0;

    bool is_pair = index.column() == index.row();
    QColor cellEmptyColor = is_pair ? RangeSelectorTableDelegate::pairColor : RangeSelectorTableDelegate::emptyColor;

    QPainterPath cellPath;
    cellPath.addRoundedRect(cellRect, radius, radius);
    painter->fillPath(cellPath, cellEmptyColor);

    float range_float = this->rangeSelectorTableModel->getRangeAt(index.row(),index.column());
    if(range_float > 0.0f){
        float fold_prob = 1 - range_float;
        int disable_height = (int)(fold_prob * cellRect.height());
        QRectF filledRect(cellRect.left(), cellRect.top() + disable_height,
                           cellRect.width(), cellRect.height() - disable_height);
        painter->save();
        painter->setClipPath(cellPath);
        painter->fillRect(filledRect, RangeSelectorTableDelegate::accentColor);
        painter->restore();
    }

    painter->setPen(QPen(RangeSelectorTableDelegate::borderColor, 1));
    painter->drawPath(cellPath);

    QTextDocument doc;
    doc.setHtml(options.text);

    painter->translate(options.rect.left(), options.rect.top());
    QRect clip(0, 0, options.rect.width(), options.rect.height());
    if(!this->rangeSelectorTableModel->in_thumbnail_mode()){
        doc.drawContents(painter, clip);
    }
    painter->restore();
}
