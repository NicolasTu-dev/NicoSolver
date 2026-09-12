#include "logindialog.h"
#include "include/data/licensemanager.h"
#include "licensedialog.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle(tr("Solverix — Iniciar sesión"));
    this->setMinimumSize(420, 380);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(14);
    layout->setContentsMargins(30, 30, 30, 30);

    QLabel* title = new QLabel(tr("♠ Solverix"), this);
    title->setStyleSheet("font-size:24px; font-weight:800;");
    layout->addWidget(title);

    QLabel* subtitle = new QLabel(tr("Iniciá sesión para usar el solver."), this);
    subtitle->setStyleSheet("font-size:13.5px; color:#8fb39f;");
    layout->addWidget(subtitle);

    QLabel* demoNote = new QLabel(tr(
        "Demo: cualquier usuario y contraseña funcionan. Lo que se verifica de "
        "verdad es si la suscripción local está activa."), this);
    demoNote->setWordWrap(true);
    demoNote->setStyleSheet("font-size:11.5px; color:#5c7a6a;");
    layout->addWidget(demoNote);

    QFrame* divider = new QFrame(this);
    divider->setFrameShape(QFrame::HLine);
    layout->addWidget(divider);

    QLabel* emailLabel = new QLabel(tr("Usuario / email"), this);
    layout->addWidget(emailLabel);
    this->emailField = new QLineEdit(this);
    layout->addWidget(this->emailField);

    QLabel* passLabel = new QLabel(tr("Contraseña"), this);
    layout->addWidget(passLabel);
    this->passwordField = new QLineEdit(this);
    this->passwordField->setEchoMode(QLineEdit::Password);
    layout->addWidget(this->passwordField);

    QPushButton* loginBtn = new QPushButton(tr("Iniciar sesión"), this);
    loginBtn->setStyleSheet("font-size:14px; font-weight:700; padding:12px 10px;");
    connect(loginBtn, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    layout->addWidget(loginBtn);

    this->statusLabel = new QLabel(this);
    this->statusLabel->setWordWrap(true);
    this->statusLabel->setStyleSheet("font-size:13px;");
    this->statusLabel->setVisible(false);
    layout->addWidget(this->statusLabel);

    QPushButton* activateBtn = new QPushButton(tr("No tengo suscripción — activar (demo)"), this);
    activateBtn->setStyleSheet("font-size:12px; padding:8px 10px;");
    connect(activateBtn, &QPushButton::clicked, this, &LoginDialog::onActivateClicked);
    layout->addWidget(activateBtn);

    layout->addStretch();
}

void LoginDialog::onLoginClicked(){
    if(this->emailField->text().trimmed().isEmpty() || this->passwordField->text().isEmpty()){
        this->statusLabel->setText(tr("Completá usuario y contraseña."));
        this->statusLabel->setStyleSheet("font-size:13px; color:#ff5c5c;");
        this->statusLabel->setVisible(true);
        return;
    }
    if(LicenseManager::isActive()){
        this->accept();
        return;
    }
    this->statusLabel->setText(tr("Tu cuenta no tiene una suscripción activa. Activá una (abajo) para entrar."));
    this->statusLabel->setStyleSheet("font-size:13px; color:#ff5c5c;");
    this->statusLabel->setVisible(true);
}

void LoginDialog::onActivateClicked(){
    LicenseDialog licenseDialog(this);
    licenseDialog.exec();
    if(LicenseManager::isActive()){
        this->statusLabel->setText(tr("¡Listo! Ya tenés una suscripción activa — tocá \"Iniciar sesión\"."));
        this->statusLabel->setStyleSheet("font-size:13px; color:#22c55e;");
        this->statusLabel->setVisible(true);
    }
}
