#ifndef APICLIENT_H
#define APICLIENT_H

#include <QString>
#include <functional>

// Talks to the real Solverix backend (Vercel + Postgres). Registering and
// activating a plan happen on the website only — the app can log in and
// re-check entitlement, nothing more.
namespace ApiClient {

struct LoginResult {
    bool ok = false;
    bool active = false;
    QString plan; // "none" | "advanced" | "complete"
    QString expiresAt; // ISO date string, or empty
    QString error;
};

void login(const QString& email, const QString& password, std::function<void(LoginResult)> callback);

// Re-checks entitlement for an already-logged-in account, no password
// needed. Used for periodic re-validation while the app is running, since
// checking only the locally cached expiry date can't detect an early
// cancellation made from elsewhere.
void checkStatus(const QString& email, std::function<void(LoginResult)> callback);

} // namespace ApiClient

#endif // APICLIENT_H
