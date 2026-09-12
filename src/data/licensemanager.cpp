#include "include/data/licensemanager.h"
#include <QSettings>
#include <QObject>

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

void activate(Plan plan, int days){
    QSettings settings = settingsStore();
    settings.setValue("plan", plan == Plan::Complete ? "complete" : "advanced");
    settings.setValue("expiry", QDate::currentDate().addDays(days));
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

} // namespace LicenseManager
