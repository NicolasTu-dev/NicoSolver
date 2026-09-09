#include "include/ui/rangeselectortablemodel.h"

RangeSelectorTableModel::RangeSelectorTableModel(QStringList ranks,QString initial_board,QObject *parent,bool thumbnail):QAbstractItemModel(parent){
    this->ranklist = ranks;
    this->thumbnail = thumbnail;

    this->grids_string = vector<vector<QString>>(this->ranklist.size());
    for(int i = 0;i < this->ranklist.size();i ++){
        this->grids_string[i] = vector<QString>(this->ranklist.size());
        for(int j = 0;j < this->ranklist.size();j ++){
            this->grids_string[i][j] = this->get_ij_text(i,j);
            this->string2ij[this->get_ij_text(i,j)] = pair<int,int>(i,j);
        }
    }

    this->grids_float = vector<vector<float>>(this->ranklist.size());
    for(int i = 0;i < this->ranklist.size();i ++){
        this->grids_float[i] = vector<float>(this->ranklist.size(),0.0);
    }
    this->setRangeText(initial_board);
}

float RangeSelectorTableModel::getRangeAt(int i, int j){
    if(i < 0 || i >= this->ranklist.size()){
        qDebug().noquote() << "RangeSelectorTableModel::getRangeAt i not valid: " << i;
        return 0;
    }
    else if(j < 0 || j >= this->ranklist.size()){
        qDebug().noquote() << "RangeSelectorTableModel::getRangeAt j not valid: " << j;
        return 0;
    }
    else{
        return this->grids_float[i][j];
    }
}

void RangeSelectorTableModel::clear_range(){
    for(int i = 0;i < this->ranklist.size();i ++){
        for(int j = 0;j < this->ranklist.size();j ++){
            this->grids_float[i][j] = 0;
        }
    }
}

void RangeSelectorTableModel::setRangeAt(int i, int j,float value){
    if(i < 0 || i >= this->ranklist.size()){
        qDebug().noquote() << "RangeSelectorTableModel::setRangeAt i not valid: " << i;
    }
    else if(j < 0 || j >= this->ranklist.size()){
        qDebug().noquote() << "RangeSelectorTableModel::setRangeAt j not valid: " << j;
    }
    else{
        this->grids_float[i][j] = value;
    }
}

RangeSelectorTableModel::~RangeSelectorTableModel(){
}

QString RangeSelectorTableModel::getRangeText(){
    QString retval = "";
    for(int i = 0;i < this->ranklist.size();i ++){
        for(int j = 0;j < this->ranklist.size();j ++){
            if(this->grids_float[i][j] == 0)continue;
            QString one_range_str = QString("%1:%2").arg(this->grids_string[i][j],QString::number(this->grids_float[i][j],'f',3));
            if(retval == ""){
                retval = one_range_str;
            }else{
                retval = retval + "," + one_range_str;
            }
        }
    }
    return retval;
}

void RangeSelectorTableModel::setRangeText(QString input_range){
    this->clear_range();
    for(QString one_range:input_range.split(",")){
        if(one_range.trimmed() == "")continue;
        QStringList one_range_list = one_range.split(":");

        if(one_range_list.size() > 2 || one_range_list.size() < 1){
            qDebug().noquote() << QString(tr("skipping range %1, format error, range list should contain two part seperate by ':'")).arg(one_range);
        }

        float range_float = 1.0;
        QString key_range = one_range_list[0];
        QStringList keys;
        if(this->string2ij.count(one_range_list[0])){
            keys.append(key_range);
        }else{
            keys.append(key_range + "o");
            keys.append(key_range + "s");
        }

        for(QString one_key: keys){
            if(one_key.count(" ") == one_key.length() || one_key == "")continue;
            if(!this->string2ij.count(one_key)){
                qDebug().noquote() << QString(tr("skipping range %1, format error")).arg(one_range);
                continue;
            }
            pair<int,int> cord = this->string2ij[one_key];
            if(one_range_list.size() > 1)range_float = one_range_list[1].toFloat();
            this->grids_float[cord.first][cord.second] = range_float;
        }
    }
}

QString RangeSelectorTableModel::get_ij_text(int i,int j){
    int row = i;
    int col = j;
    int larger = row > col?row:col;
    int smaller = row > col?col:row;
    QString  retval = QString("%1%2%3")\
            .arg(QString(this->ranklist[smaller]))\
            .arg(QString(this->ranklist[larger]));
    if(row > col){
        retval = retval.arg("o");
    }else if(row < col){
        retval = retval.arg("s");
    }else{
        retval = retval.arg("");
    }
    return retval;
}

QVariant RangeSelectorTableModel::data(const QModelIndex &index, int role) const {
    int row = index.row();
    int col = index.column();
    int larger = row > col?row:col;
    int smaller = row > col?col:row;
    QString  hand = QString("%1%2%3")\
            .arg(QString(this->ranklist[smaller]))\
            .arg(QString(this->ranklist[larger]));
    if(row > col){
        hand = hand.arg(tr("o"));
    }else if(row < col){
        hand = hand.arg(tr("s"));
    }else{
        hand = hand.arg(tr(""));
    }
    float freq = this->grids_float[row][col];
    QString retval = QString("<div style='text-align:center; color:#ffffff;'>"
                              "<span style='font-size:9pt; font-weight:600;'>%1</span>").arg(hand);
    if(freq > 0.0f){
        int pct = (int)(freq * 100 + 0.5f);
        retval += QString("<br><span style='font-size:7pt;'>%1%</span>").arg(pct);
    }
    retval += "</div>";
    return retval;
}

QModelIndex RangeSelectorTableModel::index(int row, int column,
          const QModelIndex &parent) const  {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, nullptr);
}

QVariant RangeSelectorTableModel::headerData(int section, Qt::Orientation orientation, int role){
    return QString::fromStdString("");
}

QModelIndex RangeSelectorTableModel::parent(const QModelIndex &child) const {
    return QModelIndex();
}

int RangeSelectorTableModel::rowCount(const QModelIndex &parent) const {
    return this->ranklist.size();
}

int RangeSelectorTableModel::columnCount(const QModelIndex &parent) const {
    return this->ranklist.size();
}
