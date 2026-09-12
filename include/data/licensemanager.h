#ifndef LICENSEMANAGER_H
#define LICENSEMANAGER_H

#include <QDate>
#include <QString>

// Local, offline simulation of a subscription for demo purposes.
// There is no real payment or server involved: activating a plan just
// stores a plan name and an expiry date in the app's local settings.
namespace LicenseManager {

enum class Plan { None, Advanced, Complete };

// Currently entitled plan. Returns Plan::None if never activated, or if
// the stored expiry date has already passed (subscription "ended").
Plan currentPlan();

QDate expiryDate();

// Negative once expired.
int daysRemaining();

bool isActive();

// Simulates subscribing: stores the plan and pushes the expiry `days`
// days into the future from today. No payment happens.
void activate(Plan plan, int days = 30);

// Simulates cancelling / letting the subscription lapse immediately.
void deactivate();

QString planDisplayName(Plan plan);

} // namespace LicenseManager

#endif // LICENSEMANAGER_H
