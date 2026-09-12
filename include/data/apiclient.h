#ifndef APICLIENT_H
#define APICLIENT_H

#include <QString>
#include <functional>

// Talks to the real Solverix demo backend (Vercel + Postgres). No local
// simulation left here — login/register/activate are genuine HTTP calls.
namespace ApiClient {

struct LoginResult {
    bool ok = false;
    bool active = false;
    QString plan; // "none" | "advanced" | "complete"
    QString expiresAt; // ISO date string, or empty
    QString error;
};

struct SimpleResult {
    bool ok = false;
    QString error;
};

void login(const QString& email, const QString& password, std::function<void(LoginResult)> callback);
void registerAccount(const QString& email, const QString& password, std::function<void(SimpleResult)> callback);
void activate(const QString& email, const QString& plan, std::function<void(LoginResult)> callback);

} // namespace ApiClient

#endif // APICLIENT_H
