#ifndef LICENSEDIALOG_H
#define LICENSEDIALOG_H

#include <QDialog>
#include <QLabel>

class LicenseDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LicenseDialog(QWidget *parent = 0);

private slots:
    void onActivateAdvanced();
    void onActivateComplete();
    void onDeactivate();

private:
    QLabel* statusLabel;
    void refreshStatus();
};

#endif // LICENSEDIALOG_H
