#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <QDialog>

namespace Ui {
class WelcomeDialog;
}

class WelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    enum Choice { Cancelled, QuickMode, Advanced };

    explicit WelcomeDialog(QWidget *parent = 0);
    ~WelcomeDialog();
    Choice choice();

private slots:
    void on_quickModeButton_clicked();
    void on_advancedButton_clicked();

private:
    Ui::WelcomeDialog *ui;
    Choice result = Cancelled;
};

#endif // WELCOMEDIALOG_H
