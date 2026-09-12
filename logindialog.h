#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

// First screen shown on every launch. Talks to the real Solverix demo
// backend (see ApiClient) to log in or register, then checks whether the
// account has an active subscription. No local shortcut — wrong
// credentials really fail, and only a server-confirmed active plan lets
// you in.
class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = 0);

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onActivateAdvancedClicked();
    void onActivateCompleteClicked();

private:
    QLineEdit* loginEmailField;
    QLineEdit* loginPasswordField;
    QPushButton* loginButton;
    QLabel* loginStatusLabel;
    QWidget* activationArea;
    QPushButton* activateAdvancedButton;
    QPushButton* activateCompleteButton;

    QLineEdit* registerEmailField;
    QLineEdit* registerPasswordField;
    QPushButton* registerButton;
    QLabel* registerStatusLabel;

    void attemptActivate(const QString& plan);
    void setBusy(bool busy);
};

#endif // LOGINDIALOG_H
