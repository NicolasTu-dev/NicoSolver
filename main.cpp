#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QInputDialog>
#include <string>
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QFontDatabase>
#include "include/ui/rangeselectortabledelegate.h"


void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if(MainWindow::s_textEdit == 0)
    {
        QByteArray localMsg = msg.toLocal8Bit();
        switch (type) {
        case QtDebugMsg:
            fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            break;
        case QtWarningMsg:
            fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            break;
        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            break;
        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            abort();
        }
    }
    else
    {
        // redundant check, could be removed, or the
        // upper if statement could be removed
        if(MainWindow::s_textEdit != 0){
            MainWindow::s_textEdit->log_with_signal(msg);
            MainWindow::s_textEdit->update();
        }
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);
    QApplication a(argc, argv);

    QFontDatabase::addApplicationFont(":/resources/fonts/Cinzel-Bold.ttf");
    QFontDatabase::addApplicationFont(":/resources/fonts/Inter-Regular.ttf");
    QFontDatabase::addApplicationFont(":/resources/fonts/Inter-Bold.ttf");
    QFontDatabase::addApplicationFont(":/resources/fonts/SpaceGrotesk-Regular.ttf");
    QFontDatabase::addApplicationFont(":/resources/fonts/SpaceGrotesk-Bold.ttf");
    QFontDatabase::addApplicationFont(":/resources/fonts/Manrope-Regular.ttf");
    QFontDatabase::addApplicationFont(":/resources/fonts/Manrope-Bold.ttf");

    QSettings setting("Solverix", "Setting");
    setting.beginGroup("solver");
    QString language_str = setting.value("language").toString();
    QTranslator trans;

    if(language_str == ""){
        QStringList languages;
        languages << "English" << QString::fromLocal8Bit("简体中文") << "Español";
        QString lang = QInputDialog::getItem(NULL,"select language","language",languages,0,false);

        if(lang == "English"){
            trans.load(":/lang_en.qm");
            language_str = "EN";
        }else if(lang == QString::fromLocal8Bit("简体中文")){
            trans.load(":/lang_cn.qm");
            language_str = "CN";
        }else if(lang == "Español"){
            trans.load(":/lang_es.qm");
            language_str = "ES";
        }
        a.installTranslator(&trans);
        setting.setValue("language",language_str);
    }else{
        if(language_str == "EN"){
            trans.load(":/lang_en.qm");
        }else if(language_str == "CN"){
            trans.load(":/lang_cn.qm");
        }else if(language_str == "ES"){
            trans.load(":/lang_es.qm");
        }
        a.installTranslator(&trans);
    }

    QStringList validThemes;
    validThemes << "dark" << "light" << "pokerroom" << "violet" << "fintech";
    QString theme_str = setting.value("theme").toString();
    if(!validThemes.contains(theme_str)){
        theme_str = "pokerroom";
        setting.setValue("theme", theme_str);
    }
    QString themePath = ":/resources/themes/theme_dark.qss";
    if(theme_str == "light"){
        themePath = ":/resources/themes/theme_light.qss";
    }else if(theme_str == "pokerroom"){
        themePath = ":/resources/themes/theme_pokerroom.qss";
    }else if(theme_str == "violet"){
        themePath = ":/resources/themes/theme_violet.qss";
    }else if(theme_str == "fintech"){
        themePath = ":/resources/themes/theme_fintech.qss";
    }
    QFile themeFile(themePath);
    if(themeFile.open(QFile::ReadOnly | QFile::Text)){
        QTextStream themeStream(&themeFile);
        a.setStyleSheet(themeStream.readAll());
        themeFile.close();
    }

    if(theme_str == "pokerroom"){
        RangeSelectorTableDelegate::accentColor = QColor("#d4af37");
        RangeSelectorTableDelegate::emptyColor = QColor("#132118");
        RangeSelectorTableDelegate::pairColor = QColor("#1c2f22");
        RangeSelectorTableDelegate::borderColor = QColor("#06120b");
    }else if(theme_str == "light"){
        RangeSelectorTableDelegate::accentColor = QColor("#3d5afe");
        RangeSelectorTableDelegate::emptyColor = QColor("#eef0f4");
        RangeSelectorTableDelegate::pairColor = QColor("#e2e4ea");
        RangeSelectorTableDelegate::borderColor = QColor("#ffffff");
    }else if(theme_str == "violet"){
        RangeSelectorTableDelegate::accentColor = QColor("#7c6ff0");
        RangeSelectorTableDelegate::emptyColor = QColor("#1e1c2b");
        RangeSelectorTableDelegate::pairColor = QColor("#282540");
        RangeSelectorTableDelegate::borderColor = QColor("#0d0c14");
    }else if(theme_str == "fintech"){
        RangeSelectorTableDelegate::accentColor = QColor("#00e5a0");
        RangeSelectorTableDelegate::emptyColor = QColor("#0d1017");
        RangeSelectorTableDelegate::pairColor = QColor("#161b26");
        RangeSelectorTableDelegate::borderColor = QColor("#05070d");
    }

    int dump_round = setting.value("dump_round").toInt();
    if(dump_round == 0){
        setting.setValue("dump_round",2);
    }

    MainWindow w;
    w.show();

    return a.exec();
}
