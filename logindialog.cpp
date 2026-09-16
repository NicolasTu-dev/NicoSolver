#include "logindialog.h"
#include "include/data/licensemanager.h"
#include "include/data/apiclient.h"
#include <QVBoxLayout>
#include <QFrame>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>

static const char* WEBSITE_URL = "https://solverix-nicolastu-devs-projects.vercel.app/cuenta.html";

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle(tr("Solverix Log in"));
    this->setMinimumSize(400, 320);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(14);
    layout->setContentsMargins(30, 30, 30, 30);

    QLabel* title = new QLabel(tr("♠ Solverix"), this);
    title->setStyleSheet("font-size:24px; font-weight:800;");
    layout->addWidget(title);

    layout->addWidget(new QLabel(tr("Email"), this));
    this->loginEmailField = new QLineEdit(this);
    layout->addWidget(this->loginEmailField);
    layout->addWidget(new QLabel(tr("Password"), this));
    this->loginPasswordField = new QLineEdit(this);
    this->loginPasswordField->setEchoMode(QLineEdit::Password);
    layout->addWidget(this->loginPasswordField);

    this->rememberMeCheck = new QCheckBox(tr("Remember my email and password"), this);
    this->rememberMeCheck->setChecked(true);
    layout->addWidget(this->rememberMeCheck);

    this->loginButton = new QPushButton(tr("Log in"), this);
    this->loginButton->setStyleSheet("font-size:14px; font-weight:700; padding:12px 10px;");
    connect(this->loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    layout->addWidget(this->loginButton);

    this->loginStatusLabel = new QLabel(this);
    this->loginStatusLabel->setWordWrap(true);
    this->loginStatusLabel->setStyleSheet("font-size:13px;");
    this->loginStatusLabel->setVisible(false);
    layout->addWidget(this->loginStatusLabel);

    this->noPlanArea = new QWidget(this);
    QVBoxLayout* noPlanLayout = new QVBoxLayout(this->noPlanArea);
    noPlanLayout->setContentsMargins(0, 8, 0, 0);
    QFrame* divider = new QFrame(this->noPlanArea);
    divider->setFrameShape(QFrame::HLine);
    noPlanLayout->addWidget(divider);
    noPlanLayout->addWidget(new QLabel(tr("That account doesn't have an active subscription. Get a plan on the website."), this->noPlanArea));
    QPushButton* openWebsiteButton = new QPushButton(tr("Go to the website to get a plan"), this->noPlanArea);
    connect(openWebsiteButton, &QPushButton::clicked, this, &LoginDialog::onOpenWebsiteClicked);
    noPlanLayout->addWidget(openWebsiteButton);
    this->noPlanArea->setVisible(false);
    layout->addWidget(this->noPlanArea);

    QLabel* noAccountNote = new QLabel(tr("Don't have an account? Sign up on the Solverix website."), this);
    noAccountNote->setStyleSheet("font-size:12px; color:#8fb39f;");
    noAccountNote->setWordWrap(true);
    layout->addWidget(noAccountNote);

    layout->addStretch();

    // Pre-fill (but don't auto-submit) with remembered credentials, if any.
    // Stored in plain QSettings (same as every other app setting) rather
    // than an OS credential store acceptable here since this is a
    // single-user desktop app with no other secure-storage dependency.
    QSettings setting("Solverix", "Setting");
    setting.beginGroup("solver");
    QString savedEmail = setting.value("savedEmail").toString();
    QString savedPassword = setting.value("savedPassword").toString();
    bool remember = setting.value("rememberMe", true).toBool();
    setting.endGroup();
    this->rememberMeCheck->setChecked(remember);
    if(!savedEmail.isEmpty() && !savedPassword.isEmpty()){
        this->loginEmailField->setText(savedEmail);
        this->loginPasswordField->setText(savedPassword);
    }
}

void LoginDialog::setBusy(bool busy){
    this->loginButton->setEnabled(!busy);
}

void LoginDialog::saveOrClearRememberedCredentials(const QString& email, const QString& password){
    QSettings setting("Solverix", "Setting");
    setting.beginGroup("solver");
    if(this->rememberMeCheck->isChecked()){
        setting.setValue("savedEmail", email);
        setting.setValue("savedPassword", password);
        setting.setValue("rememberMe", true);
    }else{
        setting.remove("savedEmail");
        setting.remove("savedPassword");
        setting.setValue("rememberMe", false);
    }
    setting.endGroup();
}

void LoginDialog::onLoginClicked(){
    QString email = this->loginEmailField->text().trimmed();
    QString password = this->loginPasswordField->text();
    if(email.isEmpty() || password.isEmpty()){
        this->loginStatusLabel->setText(tr("Fill in your email and password."));
        this->loginStatusLabel->setStyleSheet("font-size:13px; color:#ff5c5c;");
        this->loginStatusLabel->setVisible(true);
        return;
    }

    this->noPlanArea->setVisible(false);
    this->loginStatusLabel->setText(tr("Connecting..."));
    this->loginStatusLabel->setStyleSheet("font-size:13px; color:#8fb39f;");
    this->loginStatusLabel->setVisible(true);
    this->setBusy(true);

    ApiClient::login(email, password, [this, email, password](ApiClient::LoginResult result){
        this->setBusy(false);
        if(!result.ok){
            if(result.error == "invalid_credentials"){
                // A remembered password stopped working (changed on the
                // website, etc.) stop auto-filling it on every launch.
                QSettings setting("Solverix", "Setting");
                setting.beginGroup("solver");
                setting.remove("savedPassword");
                setting.endGroup();
            }
            QString message = (result.error == "invalid_credentials")
                ? tr("Incorrect email or password.")
                : (result.error.isEmpty() ? tr("Couldn't connect to the server.") : result.error);
            this->loginStatusLabel->setText(message);
            this->loginStatusLabel->setStyleSheet("font-size:13px; color:#ff5c5c;");
            return;
        }

        LicenseManager::setCurrentUserEmail(email);
        LicenseManager::cacheFromServer(result.plan, result.expiresAt);
        this->saveOrClearRememberedCredentials(email, password);

        if(!result.active){
            this->loginStatusLabel->setText(tr("Logged in successfully."));
            this->loginStatusLabel->setStyleSheet("font-size:13px; color:#8fb39f;");
            this->noPlanArea->setVisible(true);
            return;
        }

        this->accept();
    });
}

void LoginDialog::onOpenWebsiteClicked(){
    QDesktopServices::openUrl(QUrl(WEBSITE_URL));
}
