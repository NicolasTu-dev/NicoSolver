#include "include/data/licensemanager.h"
#include <QSettings>
#include <QObject>
#include <QDateTime>

namespace LicenseManager {

static QSettings settingsStore(){
    return QSettings("Solverix", "License");
}

Plan currentPlan(){
    QSettings settings = settingsStore();
    QString planStr = settings.value("plan", "none").toString();
    QDate expiry = settings.value("expiry").toDate();

    if(planStr == "none" || !expiry.isValid()) return Plan::None;
    if(QDate::currentDate() > expiry) return Plan::None; // subscription ended

    if(planStr == "complete") return Plan::Complete;
    if(planStr == "advanced") return Plan::Advanced;
    return Plan::None;
}

QDate expiryDate(){
    QSettings settings = settingsStore();
    return settings.value("expiry").toDate();
}

int daysRemaining(){
    QDate expiry = expiryDate();
    if(!expiry.isValid()) return 0;
    return QDate::currentDate().daysTo(expiry);
}

bool isActive(){
    return currentPlan() != Plan::None;
}

void cacheFromServer(QString planStr, QString expiresAtIso){
    QSettings settings = settingsStore();
    if(planStr == "none" || planStr.isEmpty() || expiresAtIso.isEmpty()){
        settings.setValue("plan", "none");
        settings.remove("expiry");
        return;
    }
    QDateTime expiresAt = QDateTime::fromString(expiresAtIso, Qt::ISODateWithMs);
    if(!expiresAt.isValid()) expiresAt = QDateTime::fromString(expiresAtIso, Qt::ISODate);
    settings.setValue("plan", planStr);
    settings.setValue("expiry", expiresAt.date());
}

void deactivate(){
    QSettings settings = settingsStore();
    settings.setValue("plan", "none");
    settings.remove("expiry");
}

QString planDisplayName(Plan plan){
    switch(plan){
        case Plan::Advanced: return QObject::tr("Solver Avanzado");
        case Plan::Complete: return QObject::tr("Completo");
        default: return QObject::tr("Sin suscripción");
    }
}

QString currentUserEmail(){
    QSettings settings = settingsStore();
    return settings.value("currentUserEmail").toString();
}

void setCurrentUserEmail(QString email){
    QSettings settings = settingsStore();
    settings.setValue("currentUserEmail", email);
}

} // namespace LicenseManager
