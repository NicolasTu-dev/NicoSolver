#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>

// First screen shown on every launch: a (cosmetic, demo-only) login form
// that then checks whether the "account" has an active subscription via
// LicenseManager. No real authentication happens — any non-empty email
// and password are accepted — this is purely to demo what a real gated
// login would feel like.
class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = 0);

private slots:
    void onLoginClicked();
    void onActivateClicked();

private:
    QLineEdit* emailField;
    QLineEdit* passwordField;
    QLabel* statusLabel;
};

#endif // LOGINDIALOG_H
