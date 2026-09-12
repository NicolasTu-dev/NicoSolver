#include "include/data/apiclient.h"
#include "include/data/apisecrets.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

namespace ApiClient {

static const char* BASE_URL = "https://solverix-nicolastu-devs-projects.vercel.app";

// Lets the app's HTTP calls through Vercel's deployment protection while
// the site stays gated for regular browser visitors. The actual secret
// lives in apisecrets.h, which is gitignored (see apisecrets.h.example).
static const char* PROTECTION_BYPASS = VERCEL_PROTECTION_BYPASS;

static QNetworkAccessManager* manager(){
    static QNetworkAccessManager* instance = new QNetworkAccessManager();
    return instance;
}

static QNetworkRequest buildRequest(const QString& path){
    QNetworkRequest request(QUrl(QString(BASE_URL) + path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("x-vercel-protection-bypass", PROTECTION_BYPASS);
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

void registerAccount(const QString& email, const QString& password, std::function<void(SimpleResult)> callback){
    QJsonObject body;
    body["email"] = email;
    body["password"] = password;

    QNetworkReply* reply = manager()->post(buildRequest("/api/register"), toJson(body));
    QObject::connect(reply, &QNetworkReply::finished, [reply, callback](){
        SimpleResult result;
        QByteArray data = reply->readAll();
        QJsonObject obj = QJsonDocument::fromJson(data).object();
        if(reply->error() != QNetworkReply::NoError && obj.isEmpty()){
            result.error = reply->errorString();
        }else{
            result.ok = obj.value("ok").toBool();
            result.error = obj.value("error").toString();
        }
        reply->deleteLater();
        callback(result);
    });
}

void activate(const QString& email, const QString& plan, std::function<void(LoginResult)> callback){
    QJsonObject body;
    body["email"] = email;
    body["plan"] = plan;

    QNetworkReply* reply = manager()->post(buildRequest("/api/activate"), toJson(body));
    QObject::connect(reply, &QNetworkReply::finished, [reply, callback](){
        LoginResult result;
        QByteArray data = reply->readAll();
        QJsonObject obj = QJsonDocument::fromJson(data).object();
        if(reply->error() != QNetworkReply::NoError && obj.isEmpty()){
            result.error = reply->errorString();
        }else{
            result.ok = obj.value("ok").toBool();
            result.active = result.ok;
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
