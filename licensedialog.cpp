#include "licensedialog.h"
#include "include/data/licensemanager.h"
#include "include/data/apiclient.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QDesktopServices>
#include <QUrl>

static const char* WEBSITE_URL = "https://solverix-nicolastu-devs-projects.vercel.app/cuenta.html";

LicenseDialog::LicenseDialog(QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle(tr("My subscription"));
    this->setMinimumSize(440, 300);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(16);
    layout->setContentsMargins(28, 28, 28, 28);

    QLabel* title = new QLabel(tr("🔑 My subscription"), this);
    title->setStyleSheet("font-size:20px; font-weight:800;");
    layout->addWidget(title);

    QLabel* accountNote = new QLabel(tr("Account: %1").arg(LicenseManager::currentUserEmail()), this);
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

    QPushButton* openWebsiteBtn = new QPushButton(tr("Go to the website to get or renew a plan"), this);
    openWebsiteBtn->setStyleSheet("font-size:14px; font-weight:700; padding:12px 10px;");
    connect(openWebsiteBtn, &QPushButton::clicked, this, &LicenseDialog::onOpenWebsite);
    layout->addWidget(openWebsiteBtn);

    QPushButton* recheckBtn = new QPushButton(tr("I already got a plan check again"), this);
    recheckBtn->setStyleSheet("font-size:12.5px; padding:10px 10px;");
    connect(recheckBtn, &QPushButton::clicked, this, &LicenseDialog::onRecheckStatus);
    layout->addWidget(recheckBtn);

    QPushButton* deactivateBtn = new QPushButton(tr("Log out"), this);
    deactivateBtn->setStyleSheet("font-size:12.5px; padding:8px 10px;");
    connect(deactivateBtn, &QPushButton::clicked, this, &LicenseDialog::onDeactivate);
    layout->addWidget(deactivateBtn);

    this->refreshStatus();
}

void LicenseDialog::refreshStatus(){
    LicenseManager::Plan plan = LicenseManager::currentPlan();
    if(plan == LicenseManager::Plan::None){
        this->statusLabel->setText(tr("No active subscription. Get a plan on the website to keep using the solver."));
        this->statusLabel->setStyleSheet("font-size:15px; font-weight:700; color:#ff5c5c;");
    }else{
        int days = LicenseManager::daysRemaining();
        this->statusLabel->setText(tr("Plan %1 active. Expires in %2 day(s) (%3).")
            .arg(LicenseManager::planDisplayName(plan))
            .arg(days)
            .arg(LicenseManager::expiryDate().toString("dd/MM/yyyy")));
        this->statusLabel->setStyleSheet("font-size:15px; font-weight:700; color:#22c55e;");
    }
}

void LicenseDialog::onOpenWebsite(){
    QDesktopServices::openUrl(QUrl(WEBSITE_URL));
}

void LicenseDialog::onRecheckStatus(){
    QString email = LicenseManager::currentUserEmail();
    if(email.isEmpty()) return;
    this->statusLabel->setText(tr("Checking..."));
    this->statusLabel->setStyleSheet("font-size:15px; font-weight:600;");
    ApiClient::checkStatus(email, [this](ApiClient::LoginResult result){
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
