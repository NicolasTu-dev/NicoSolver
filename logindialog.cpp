#include "logindialog.h"
#include "include/data/licensemanager.h"
#include "include/data/apiclient.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QFrame>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle(tr("Solverix — Iniciar sesión"));
    this->setMinimumSize(440, 480);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(14);
    layout->setContentsMargins(30, 30, 30, 30);

    QLabel* title = new QLabel(tr("♠ Solverix"), this);
    title->setStyleSheet("font-size:24px; font-weight:800;");
    layout->addWidget(title);

    QTabWidget* tabs = new QTabWidget(this);

    // --- Login tab ---
    QWidget* loginTab = new QWidget(this);
    QVBoxLayout* loginLayout = new QVBoxLayout(loginTab);
    loginLayout->addWidget(new QLabel(tr("Email"), loginTab));
    this->loginEmailField = new QLineEdit(loginTab);
    loginLayout->addWidget(this->loginEmailField);
    loginLayout->addWidget(new QLabel(tr("Contraseña"), loginTab));
    this->loginPasswordField = new QLineEdit(loginTab);
    this->loginPasswordField->setEchoMode(QLineEdit::Password);
    loginLayout->addWidget(this->loginPasswordField);

    this->loginButton = new QPushButton(tr("Iniciar sesión"), loginTab);
    this->loginButton->setStyleSheet("font-size:14px; font-weight:700; padding:12px 10px;");
    connect(this->loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    loginLayout->addWidget(this->loginButton);

    this->loginStatusLabel = new QLabel(loginTab);
    this->loginStatusLabel->setWordWrap(true);
    this->loginStatusLabel->setStyleSheet("font-size:13px;");
    this->loginStatusLabel->setVisible(false);
    loginLayout->addWidget(this->loginStatusLabel);

    this->activationArea = new QWidget(loginTab);
    QVBoxLayout* activationLayout = new QVBoxLayout(this->activationArea);
    activationLayout->setContentsMargins(0, 8, 0, 0);
    QFrame* divider = new QFrame(this->activationArea);
    divider->setFrameShape(QFrame::HLine);
    activationLayout->addWidget(divider);
    activationLayout->addWidget(new QLabel(tr("No tenés una suscripción activa. Activá una (demo, sin cobro real):"), this->activationArea));
    this->activateAdvancedButton = new QPushButton(tr("Activar Solver Avanzado (30 días)"), this->activationArea);
    this->activateCompleteButton = new QPushButton(tr("Activar Completo (30 días)"), this->activationArea);
    connect(this->activateAdvancedButton, &QPushButton::clicked, this, &LoginDialog::onActivateAdvancedClicked);
    connect(this->activateCompleteButton, &QPushButton::clicked, this, &LoginDialog::onActivateCompleteClicked);
    activationLayout->addWidget(this->activateAdvancedButton);
    activationLayout->addWidget(this->activateCompleteButton);
    this->activationArea->setVisible(false);
    loginLayout->addWidget(this->activationArea);

    loginLayout->addStretch();
    tabs->addTab(loginTab, tr("Iniciar sesión"));

    // --- Register tab ---
    QWidget* registerTab = new QWidget(this);
    QVBoxLayout* registerLayout = new QVBoxLayout(registerTab);
    registerLayout->addWidget(new QLabel(tr("Email"), registerTab));
    this->registerEmailField = new QLineEdit(registerTab);
    registerLayout->addWidget(this->registerEmailField);
    registerLayout->addWidget(new QLabel(tr("Contraseña (mínimo 4 caracteres)"), registerTab));
    this->registerPasswordField = new QLineEdit(registerTab);
    this->registerPasswordField->setEchoMode(QLineEdit::Password);
    registerLayout->addWidget(this->registerPasswordField);

    this->registerButton = new QPushButton(tr("Registrarme"), registerTab);
    this->registerButton->setStyleSheet("font-size:14px; font-weight:700; padding:12px 10px;");
    connect(this->registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
    registerLayout->addWidget(this->registerButton);

    this->registerStatusLabel = new QLabel(registerTab);
    this->registerStatusLabel->setWordWrap(true);
    this->registerStatusLabel->setStyleSheet("font-size:13px;");
    this->registerStatusLabel->setVisible(false);
    registerLayout->addWidget(this->registerStatusLabel);

    registerLayout->addStretch();
    tabs->addTab(registerTab, tr("Registrarme"));

    layout->addWidget(tabs);
}

void LoginDialog::setBusy(bool busy){
    this->loginButton->setEnabled(!busy);
    this->registerButton->setEnabled(!busy);
}

void LoginDialog::onLoginClicked(){
    QString email = this->loginEmailField->text().trimmed();
    QString password = this->loginPasswordField->text();
    if(email.isEmpty() || password.isEmpty()){
        this->loginStatusLabel->setText(tr("Completá email y contraseña."));
        this->loginStatusLabel->setStyleSheet("font-size:13px; color:#ff5c5c;");
        this->loginStatusLabel->setVisible(true);
        return;
    }

    this->activationArea->setVisible(false);
    this->loginStatusLabel->setText(tr("Conectando..."));
    this->loginStatusLabel->setStyleSheet("font-size:13px; color:#8fb39f;");
    this->loginStatusLabel->setVisible(true);
    this->setBusy(true);

    ApiClient::login(email, password, [this, email](ApiClient::LoginResult result){
        this->setBusy(false);
        if(!result.ok){
            QString message = (result.error == "invalid_credentials")
                ? tr("Email o contraseña incorrectos.")
                : (result.error.isEmpty() ? tr("No se pudo conectar con el servidor.") : result.error);
            this->loginStatusLabel->setText(message);
            this->loginStatusLabel->setStyleSheet("font-size:13px; color:#ff5c5c;");
            return;
        }

        LicenseManager::setCurrentUserEmail(email);
        LicenseManager::cacheFromServer(result.plan, result.expiresAt);

        if(!result.active){
            this->loginStatusLabel->setText(tr("Iniciaste sesión, pero tu cuenta no tiene una suscripción activa."));
            this->loginStatusLabel->setStyleSheet("font-size:13px; color:#ff5c5c;");
            this->activationArea->setVisible(true);
            return;
        }

        this->accept();
    });
}

void LoginDialog::onRegisterClicked(){
    QString email = this->registerEmailField->text().trimmed();
    QString password = this->registerPasswordField->text();
    if(email.isEmpty() || password.isEmpty()){
        this->registerStatusLabel->setText(tr("Completá email y contraseña."));
        this->registerStatusLabel->setStyleSheet("font-size:13px; color:#ff5c5c;");
        this->registerStatusLabel->setVisible(true);
        return;
    }

    this->registerStatusLabel->setText(tr("Creando la cuenta..."));
    this->registerStatusLabel->setStyleSheet("font-size:13px; color:#8fb39f;");
    this->registerStatusLabel->setVisible(true);
    this->setBusy(true);

    ApiClient::registerAccount(email, password, [this, email, password](ApiClient::SimpleResult result){
        this->setBusy(false);
        if(!result.ok){
            QString message = (result.error == "email_already_registered")
                ? tr("Ese email ya está registrado.")
                : (result.error == "password_too_short")
                    ? tr("La contraseña tiene que tener al menos 4 caracteres.")
                    : (result.error.isEmpty() ? tr("No se pudo conectar con el servidor.") : result.error);
            this->registerStatusLabel->setText(message);
            this->registerStatusLabel->setStyleSheet("font-size:13px; color:#ff5c5c;");
            return;
        }

        this->registerStatusLabel->setText(tr("¡Cuenta creada! Andá a \"Iniciar sesión\"."));
        this->registerStatusLabel->setStyleSheet("font-size:13px; color:#22c55e;");
        this->loginEmailField->setText(email);
        this->loginPasswordField->setText(password);
    });
}

void LoginDialog::attemptActivate(const QString& plan){
    QString email = this->loginEmailField->text().trimmed();
    if(email.isEmpty()) return;

    this->loginStatusLabel->setText(tr("Activando..."));
    this->loginStatusLabel->setStyleSheet("font-size:13px; color:#8fb39f;");
    this->setBusy(true);

    ApiClient::activate(email, plan, [this, email](ApiClient::LoginResult result){
        this->setBusy(false);
        if(!result.ok){
            this->loginStatusLabel->setText(result.error.isEmpty() ? tr("No se pudo activar.") : result.error);
            this->loginStatusLabel->setStyleSheet("font-size:13px; color:#ff5c5c;");
            return;
        }
        LicenseManager::setCurrentUserEmail(email);
        LicenseManager::cacheFromServer(result.plan, result.expiresAt);
        this->accept();
    });
}

void LoginDialog::onActivateAdvancedClicked(){ this->attemptActivate("advanced"); }
void LoginDialog::onActivateCompleteClicked(){ this->attemptActivate("complete"); }
