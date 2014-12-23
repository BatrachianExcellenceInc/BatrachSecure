// vim: ts=4 sw=4 fenc=utf-8
#include <QCoreApplication>
#include <QDebug>
#include <QEventLoop>
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

ApiClient::ApiClient(QString baseUrl, QObject *parent) : QObject(parent) {
    // First client configuration
    this->conf = new ClientConf(QString(".btxsec"));

    // Then the urls
    if (!baseUrl.endsWith("/")) baseUrl += "/";

    this->baseUrl = baseUrl;

    QObject::connect(&this->mgr, &QNetworkAccessManager::finished, this, &ApiClient::handleNetworkResponse);
}

QUrl ApiClient::genPath(QString path) {
    if (!path.startsWith("/")) path = "/" + path;

    QUrl url = baseUrl + QString("v") + QString::number(ApiClient::apiVersion) + path;
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

    if (res)
        handleResponse(res);
    else
        QCoreApplication::quit();
}

void ApiClient::handleResponse(ApiResponse *res) {
    res->validate();
    qDebug() << res->resCode << res->resBody;
    delete res;

    QCoreApplication::quit();
}

void ApiClient::getVerificationCode(QString transport, QString number) {
    // Hard-code for the api
    QString baseResource("accounts");
    QString resource("code");

    // Number contains +
    number = QUrl::toPercentEncoding(number);

    QString codePath = QString(baseResource) + QString("/") + QString(transport) + QString("/code/") + number;
    QUrl codeUrl = this->genPath(codePath);

    // Everything is handled by signals from this point on
    this->getResponse(codeUrl);
}

ApiClient::~ApiClient() {
    delete this->conf;
    qDebug("bye");
}

