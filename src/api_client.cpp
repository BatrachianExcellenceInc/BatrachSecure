// vim: ts=4 sw=4 fenc=utf-8
#include <QCoreApplication>
#include <QDebug>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QString>
#include <QUrl>
#include "api_client.h"

using namespace BtxSecurity;

// FIXME: See ApiResponse constructor. Horrible way to handle this.
QMap<int, const char *> TEXTSECURE_ERRORS;

/*
 * EXCEPTION
 */

ApiException::ApiException(QString msg) : BtxException(msg){
}

/*
 * RESPONSE
 */

ApiResponse::ApiResponse(QObject *parent, int resCode, QString resBody) : QObject(parent) {
    // FIXME: This needs to be something sane. Anyone who knows c++ can give a hand.
    TEXTSECURE_ERRORS[200] = NULL;
    TEXTSECURE_ERRORS[204] = NULL;

    TEXTSECURE_ERRORS[400] = "Bad number or badly formatted tokens";
    TEXTSECURE_ERRORS[422] = "Invalid transport";

    // These may be crap
    TEXTSECURE_ERRORS[401] = "Badly formatted basic_auth or invalid credentials";
    TEXTSECURE_ERRORS[403] = "Incorrect verification_code";
    TEXTSECURE_ERRORS[404] = "Unknown/unregistered number";
    TEXTSECURE_ERRORS[409] = "Mismatched devices";
    TEXTSECURE_ERRORS[410] = "Stale devices";
    TEXTSECURE_ERRORS[413] = "Rate limit exceeded";
    TEXTSECURE_ERRORS[415] = "Invalid transport or bad json";
    TEXTSECURE_ERRORS[417] = "Number already registered";

    this->resCode = resCode;
    this->resBody = resBody;
}

void ApiResponse::validate() {
    QString errorMessage;
    if (!TEXTSECURE_ERRORS.contains(this->resCode)) {
        errorMessage = QString("Unknown return code ") + QString::number(this->resCode);
    } else {
        errorMessage = TEXTSECURE_ERRORS[this->resCode];
    }

    if (errorMessage.length()) throw ApiException(errorMessage);
}

ApiResponse::~ApiResponse() {
    qDebug("destroy response");
}

/*
 * CLIENT
 */

ApiClient::ApiClient(ClientConf *conf, QObject *parent) : QObject(parent) {
    this->conf = conf;
    QObject::connect(&this->mgr, &QNetworkAccessManager::finished, this, &ApiClient::handleNetworkResponse);
}

QUrl ApiClient::genPath(QString path) {
    if (!path.startsWith("/")) path = "/" + path;

    QUrl url = this->conf->getBaseUrl() + QString("v") + QString::number(ApiClient::apiVersion) + path;
    qDebug() << url.toString() << "\n";

    return url;
}

void ApiClient::printSslErrors(QList<QSslError> errors) {
    QSslError error;
    foreach (error, errors) {
        qDebug() << "\t" << error.error();
    }
}

void ApiClient::getResponse(QUrl url) {
    // Request and get from manager
    qDebug() << "getResponse" << url.toString();
    QNetworkRequest req(url);
    QNetworkReply *reply = this->mgr.get(req);

    // XXX: This is where the ssl must be ignored for it to work
    // but we haven't told the user yet what is ignored o_O
    // Potentially nasty
    reply->ignoreSslErrors();
    QObject::connect(reply, &QNetworkReply::sslErrors, this, &ApiClient::printSslErrors);

    // signal handler picks up from here
}

void ApiClient::putResponse(QUrl url, QString headerName, QString headerValue) {
    // Request and get from manager
    qDebug() << "getResponse" << url.toString();
    QNetworkRequest req(url);
    req.setRawHeader("Content-Type", "application/json");

    QByteArray hName;
    hName.append(headerName);
    QByteArray hValue;
    hValue.append(headerValue);
    req.setRawHeader(hName, hValue);

    QJsonDocument wrap;
    QJsonObject object;

    QString b64SignalingKey(this->conf->getSignalingKey().toBase64());
    object["signalingKey"] = b64SignalingKey;
    object["supportsSms"] = false;
    object["registrationId"] = QString::number(this->conf->getRegistrationId());

    wrap.setObject(object);

    QNetworkReply *reply = this->mgr.put(req, wrap.toJson());

    // XXX: This is where the ssl must be ignored for it to work
    // but we haven't told the user yet what is ignored o_O
    // Potentially nasty
    reply->ignoreSslErrors();
    QObject::connect(reply, &QNetworkReply::sslErrors, this, &ApiClient::printSslErrors);

    // signal handler picks up from here
}

void ApiClient::handleNetworkResponse(QNetworkReply *reply) {
    ApiResponse *res = NULL;
    switch (reply->error()) {
        // These are handled as API responses, though they're not ok http
        case QNetworkReply::NoError:
        case QNetworkReply::ProtocolInvalidOperationError:
        case QNetworkReply::UnknownContentError:
            res = new ApiResponse();
            res->resCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            res->resBody = reply->readAll();
            break;
        // TODO: Prompt the user about this
        case QNetworkReply::SslHandshakeFailedError:
            qWarning() << "SSL errors";
        default:
            qWarning() << "Network error" << reply->error();
    }

    if (res) {
        handleResponse(res);
    } else {
        QCoreApplication::quit();
    }
}

void ApiClient::handleResponse(ApiResponse *res) {
    res->validate();
    qDebug() << res->resCode << res->resBody;
    delete res;

    QCoreApplication::quit();
}

void ApiClient::getVerificationCode(QString transport) {
    // Hard-code for the api
    QString baseResource("accounts");
    QString resource("code");

    // Number contains +
    QString number = QUrl::toPercentEncoding(this->conf->getNumber());

    QString codePath = QString(baseResource) + QString("/") + QString(transport) + QString("/code/") + number;
    QUrl codeUrl = this->genPath(codePath);

    // Everything is handled by signals from this point on
    this->getResponse(codeUrl);
}

void ApiClient::confirmVerificationCode(QString code) {
    QString baseResource("accounts");
    QString resource("code");

    // The code may require fixing
    // This is the obvious case, don't care about others
    code.remove("-");

    // Basic auth Base64(number:password) where password is random 16-byte ASCII
    QByteArray authArray;
    authArray.append(this->conf->getNumber() + ":" + this->conf->getPassword());
    QString basicAuth("Basic " + authArray.toBase64());

    QString path = QString(baseResource) + QString("/") + resource + QString("/") + code;
    QUrl url = this->genPath(path);

    this->putResponse(url, "Authorization", basicAuth);
}

ApiClient::~ApiClient() {
    delete this->conf;
    qDebug("bye");
}

