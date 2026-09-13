#include "include/data/apiclient.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

namespace ApiClient {

// Separate, unprotected Vercel project — it has no landing page, only the
// register/login/activate/status functions, secured by real hashed
// passwords instead of Vercel's deployment protection (which stays on for
// the marketing site, a different project).
static const char* BASE_URL = "https://solverix-api-nicolastu-devs-projects.vercel.app";

static QNetworkAccessManager* manager(){
    static QNetworkAccessManager* instance = new QNetworkAccessManager();
    return instance;
}

static QNetworkRequest buildRequest(const QString& path){
    QNetworkRequest request(QUrl(QString(BASE_URL) + path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return request;
}

static QByteArray toJson(const QJsonObject& obj){
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

void login(const QString& email, const QString& password, std::function<void(LoginResult)> callback){
    QJsonObject body;
    body["email"] = email;
    body["password"] = password;

    QNetworkReply* reply = manager()->post(buildRequest("/api/login"), toJson(body));
    QObject::connect(reply, &QNetworkReply::finished, [reply, callback](){
        LoginResult result;
        QByteArray data = reply->readAll();
        QJsonObject obj = QJsonDocument::fromJson(data).object();
        if(reply->error() != QNetworkReply::NoError && obj.isEmpty()){
            result.error = reply->errorString();
        }else{
            result.ok = obj.value("ok").toBool();
            result.active = obj.value("active").toBool();
            result.plan = obj.value("plan").toString("none");
            result.expiresAt = obj.value("expiresAt").toString();
            result.error = obj.value("error").toString();
        }
        reply->deleteLater();
        callback(result);
    });
}

void checkStatus(const QString& email, std::function<void(LoginResult)> callback){
    QJsonObject body;
    body["email"] = email;

    QNetworkReply* reply = manager()->post(buildRequest("/api/status"), toJson(body));
    QObject::connect(reply, &QNetworkReply::finished, [reply, callback](){
        LoginResult result;
        QByteArray data = reply->readAll();
        QJsonObject obj = QJsonDocument::fromJson(data).object();
        if(reply->error() != QNetworkReply::NoError && obj.isEmpty()){
            result.error = reply->errorString();
        }else{
            result.ok = obj.value("ok").toBool();
            result.active = obj.value("active").toBool();
            result.plan = obj.value("plan").toString("none");
            result.expiresAt = obj.value("expiresAt").toString();
            result.error = obj.value("error").toString();
        }
        reply->deleteLater();
        callback(result);
    });
}

} // namespace ApiClient
