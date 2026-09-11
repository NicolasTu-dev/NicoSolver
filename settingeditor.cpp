#include "settingeditor.h"
#include "ui_settingeditor.h"

SettingEditor::SettingEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingEditor)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("Settings"));
    QSettings setting("Solverix", "Setting");
    setting.beginGroup("solver");
    QString language_str = setting.value("language").toString();
    if(language_str == "EN"){
        this->ui->languageBox->setCurrentIndex(0);
    }else if(language_str == "CN"){
        this->ui->languageBox->setCurrentIndex(1);
    }else if(language_str == "ES"){
        this->ui->languageBox->setCurrentIndex(2);
    }else{
        qDebug().noquote() << tr("Unknown language: ") << language_str << tr("Setting fail");
    }

    QString theme_str = setting.value("theme").toString();
    if(theme_str == "dark"){
        this->ui->themeBox->setCurrentIndex(0);
    }else if(theme_str == "light"){
        this->ui->themeBox->setCurrentIndex(1);
    }else if(theme_str == "pokerroom"){
        this->ui->themeBox->setCurrentIndex(2);
    }else if(theme_str == "violet"){
        this->ui->themeBox->setCurrentIndex(3);
    }else if(theme_str == "fintech"){
        this->ui->themeBox->setCurrentIndex(4);
    }else{
        qDebug().noquote() << tr("Unknown theme: ") << theme_str << tr("Setting fail");
    }

    int dump_round = setting.value("dump_round").toInt();
    if(dump_round > 0 && dump_round < 4){
        this->ui->roundBox->setCurrentIndex(dump_round - 1);
    }else{
        qDebug().noquote() << tr("dump round error: ") << dump_round;
    }

    this->initial_language_index = this->ui->languageBox->currentIndex();
    this->initial_theme_index = this->ui->themeBox->currentIndex();
    this->initized = true;
}

SettingEditor::~SettingEditor()
{
    delete ui;
}

void SettingEditor::on_confirmBox_accepted()
{
    QSettings setting("Solverix", "Setting");
    setting.beginGroup("solver");
    int lang_index = this->ui->languageBox->currentIndex();
    QString language_str;
    if(lang_index == 0){
        language_str = "EN";
    }else if(lang_index == 1){
        language_str = "CN";
    }else if(lang_index == 2){
        language_str = "ES";
    }else{
        qDebug().noquote() << tr("Unknown language index: ") << lang_index << tr("Setting fail");
    }
    setting.setValue("language",language_str);

    int theme_index = this->ui->themeBox->currentIndex();
    QString theme_str;
    if(theme_index == 0){
        theme_str = "dark";
    }else if(theme_index == 1){
        theme_str = "light";
    }else if(theme_index == 2){
        theme_str = "pokerroom";
    }else if(theme_index == 3){
        theme_str = "violet";
    }else if(theme_index == 4){
        theme_str = "fintech";
    }else{
        qDebug().noquote() << tr("Unknown theme index: ") << theme_index << tr("Setting fail");
    }
    setting.setValue("theme",theme_str);

    int round = this->ui->roundBox->currentText().toInt();
    setting.setValue("dump_round",round);

    bool language_changed = lang_index != this->initial_language_index;
    bool theme_changed = theme_index != this->initial_theme_index;
    if(language_changed || theme_changed){
        QString message = tr("Restart the program for your changes to take effect.");
        qDebug().noquote() << message;
        QMessageBox msgBox;
        msgBox.setText(message);
        msgBox.exec();
    }
}
