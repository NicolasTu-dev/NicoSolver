#ifndef LICENSEMANAGER_H
#define LICENSEMANAGER_H

#include <QDate>
#include <QString>

// Caches the plan/expiry the real Solverix backend returned at login time
// (see ApiClient), so the rest of the app doesn't need to hit the network
// again just to check entitlement mid-session.
namespace LicenseManager {

enum class Plan { None, Advanced, Complete };

// Currently entitled plan. Returns Plan::None if never logged in, or if
// the cached expiry date has already passed (subscription "ended").
Plan currentPlan();

QDate expiryDate();

// Negative once expired.
int daysRemaining();

bool isActive();

// Caches what the server said at login/activate time. planStr is
// "none" | "advanced" | "complete"; expiresAtIso is an ISO-8601 date/time
// string from the API response, or empty if there's no active plan.
void cacheFromServer(QString planStr, QString expiresAtIso);

// Clears the cached entitlement (e.g. on logout).
void deactivate();

QString planDisplayName(Plan plan);

// The email address the app is currently logged in as, for follow-up API
// calls (like activating a plan) without re-asking the user.
QString currentUserEmail();
void setCurrentUserEmail(QString email);

} // namespace LicenseManager

#endif // LICENSEMANAGER_H
