#include "licensedialog.h"
#include "include/data/licensemanager.h"
#include "include/data/apiclient.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>

LicenseDialog::LicenseDialog(QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle(tr("Mi suscripción"));
    this->setMinimumSize(460, 340);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(16);
    layout->setContentsMargins(28, 28, 28, 28);

    QLabel* title = new QLabel(tr("🔑 Mi suscripción"), this);
    title->setStyleSheet("font-size:20px; font-weight:800;");
    layout->addWidget(title);

    QLabel* accountNote = new QLabel(tr("Cuenta: %1").arg(LicenseManager::currentUserEmail()), this);
    accountNote->setStyleSheet("font-size:12.5px; color:#8fb39f;");
    layout->addWidget(accountNote);

    QFrame* divider = new QFrame(this);
    divider->setFrameShape(QFrame::HLine);
    layout->addWidget(divider);

    this->statusLabel = new QLabel(this);
    this->statusLabel->setWordWrap(true);
    this->statusLabel->setStyleSheet("font-size:15px; font-weight:600;");
    layout->addWidget(this->statusLabel);

    layout->addStretch();

    QPushButton* activateAdvancedBtn = new QPushButton(tr("Activar Solver Avanzado (30 días)"), this);
    QPushButton* activateCompleteBtn = new QPushButton(tr("Activar Completo (30 días)"), this);
    QString bigStyle = "font-size:14px; font-weight:700; padding:12px 10px;";
    activateAdvancedBtn->setStyleSheet(bigStyle);
    activateCompleteBtn->setStyleSheet(bigStyle);
    connect(activateAdvancedBtn, &QPushButton::clicked, this, &LicenseDialog::onActivateAdvanced);
    connect(activateCompleteBtn, &QPushButton::clicked, this, &LicenseDialog::onActivateComplete);
    layout->addWidget(activateAdvancedBtn);
    layout->addWidget(activateCompleteBtn);

    QPushButton* deactivateBtn = new QPushButton(tr("Cerrar sesión"), this);
    deactivateBtn->setStyleSheet("font-size:12.5px; padding:8px 10px;");
    connect(deactivateBtn, &QPushButton::clicked, this, &LicenseDialog::onDeactivate);
    layout->addWidget(deactivateBtn);

    this->refreshStatus();
}

void LicenseDialog::refreshStatus(){
    LicenseManager::Plan plan = LicenseManager::currentPlan();
    if(plan == LicenseManager::Plan::None){
        this->statusLabel->setText(tr("Sin suscripción activa. Activá un plan de prueba abajo para seguir usando el solver."));
        this->statusLabel->setStyleSheet("font-size:15px; font-weight:700; color:#ff5c5c;");
    }else{
        int days = LicenseManager::daysRemaining();
        this->statusLabel->setText(tr("Plan %1 — activo. Vence en %2 día(s) (%3).")
            .arg(LicenseManager::planDisplayName(plan))
            .arg(days)
            .arg(LicenseManager::expiryDate().toString("dd/MM/yyyy")));
        this->statusLabel->setStyleSheet("font-size:15px; font-weight:700; color:#22c55e;");
    }
}

void LicenseDialog::onActivateAdvanced(){
    QString email = LicenseManager::currentUserEmail();
    this->statusLabel->setText(tr("Activando..."));
    ApiClient::activate(email, "advanced", [this](ApiClient::LoginResult result){
        if(result.ok){
            LicenseManager::cacheFromServer(result.plan, result.expiresAt);
        }
        this->refreshStatus();
    });
}

void LicenseDialog::onActivateComplete(){
    QString email = LicenseManager::currentUserEmail();
    this->statusLabel->setText(tr("Activando..."));
    ApiClient::activate(email, "complete", [this](ApiClient::LoginResult result){
        if(result.ok){
            LicenseManager::cacheFromServer(result.plan, result.expiresAt);
        }
        this->refreshStatus();
    });
}

void LicenseDialog::onDeactivate(){
    LicenseManager::deactivate();
    this->refreshStatus();
    this->close();
}
