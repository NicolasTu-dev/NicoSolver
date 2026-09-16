#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>

// First screen shown on every launch. Only lets you log in with an
// existing account — registering and activating a plan happen on the
// Solverix website, never inside the app. If the account has no active
// subscription, this shows a button that opens the website instead of
// letting you in.
class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = 0);

private slots:
    void onLoginClicked();
    void onOpenWebsiteClicked();

private:
    QLineEdit* loginEmailField;
    QLineEdit* loginPasswordField;
    QCheckBox* rememberMeCheck;
    QPushButton* loginButton;
    QLabel* loginStatusLabel;
    QWidget* noPlanArea;

    void setBusy(bool busy);
    void saveOrClearRememberedCredentials(const QString& email, const QString& password);
};

#endif // LOGINDIALOG_H
