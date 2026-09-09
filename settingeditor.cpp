#include "settingeditor.h"
#include "ui_settingeditor.h"

SettingEditor::SettingEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingEditor)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("Settings"));
    QSettings setting("TexasSolver", "Setting");
    setting.beginGroup("solver");
    QString language_str = setting.value("language").toString();
    if(language_str == "EN"){
        this->ui->languageBox->setCurrentIndex(0);
    }else if(language_str == "CN"){
        this->ui->languageBox->setCurrentIndex(1);
    }else{
        qDebug().noquote() << tr("Unknown language: ") << language_str << tr("Setting fail");
    }

    QString theme_str = setting.value("theme").toString();
    if(theme_str == "dark"){
        this->ui->themeBox->setCurrentIndex(0);
    }else if(theme_str == "light"){
        this->ui->themeBox->setCurrentIndex(1);
    }else{
        qDebug().noquote() << tr("Unknown theme: ") << theme_str << tr("Setting fail");
    }

    int dump_round = setting.value("dump_round").toInt();
    if(dump_round > 0 && dump_round < 4){
        this->ui->roundBox->setCurrentIndex(dump_round - 1);
    }else{
        qDebug().noquote() << tr("dump round error: ") << dump_round;
    }

    this->initized = true;
}

SettingEditor::~SettingEditor()
{
    delete ui;
}

void SettingEditor::on_confirmBox_accepted()
{
    QSettings setting("TexasSolver", "Setting");
    setting.beginGroup("solver");
    QString lang = this->ui->languageBox->currentText();
    QString language_str;
    if(lang == "English"){
        language_str = "EN";
    }else if(lang == "Chinese"){
        language_str = "CN";
    }else{
        qDebug().noquote() << tr("Unknown language: ") << lang << tr("Setting fail");
    }
    setting.setValue("language",language_str);

    QString theme = this->ui->themeBox->currentText();
    QString theme_str;
    if(theme == "Dark"){
        theme_str = "dark";
    }else if(theme == "Light"){
        theme_str = "light";
    }else{
        qDebug().noquote() << tr("Unknown theme: ") << theme << tr("Setting fail");
    }
    setting.setValue("theme",theme_str);

    int round = this->ui->roundBox->currentText().toInt();
    setting.setValue("dump_round",round);
}

void SettingEditor::on_languageBox_currentIndexChanged(int index)
{
    if(!initized)return;
    QString message = tr("Restart program to make language selection effective.");
    qDebug().noquote() << message;
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.exec();
}

void SettingEditor::on_themeBox_currentIndexChanged(int index)
{
    if(!initized)return;
    QString message = tr("Restart program to make theme selection effective.");
    qDebug().noquote() << message;
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.exec();
}
