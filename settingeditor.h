#ifndef SETTINGEDITOR_H
#define SETTINGEDITOR_H

#include <QDialog>
#include <QDebug>
#include <QSettings>
#include <QMessageBox>

namespace Ui {
class SettingEditor;
}

class SettingEditor : public QDialog
{
    Q_OBJECT

public:
    explicit SettingEditor(QWidget *parent = 0);
    ~SettingEditor();

private slots:
    void on_confirmBox_accepted();
    void on_createShortcutButton_clicked();

private:
    Ui::SettingEditor *ui;
    bool initized = false;
    int initial_language_index = -1;
    int initial_theme_index = -1;
};

#endif // SETTINGEDITOR_H
