#ifndef LICENSEDIALOG_H
#define LICENSEDIALOG_H

#include <QDialog>
#include <QLabel>

// Reached mid-session (e.g. when a feature needs a higher plan, or the
// subscription expired). Only shows status, a link to the website to
// buy/renew a plan, a way to re-check status after paying, and logout
// activating a plan happens on the website, never here.
class LicenseDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LicenseDialog(QWidget *parent = 0);

private slots:
    void onOpenWebsite();
    void onRecheckStatus();
    void onDeactivate();

private:
    QLabel* statusLabel;
    void refreshStatus();
};

#endif // LICENSEDIALOG_H
