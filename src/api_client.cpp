// vim: ts=4 sw=4 fenc=utf-8
#include <QDebug>
#include <QMap>
#include <QString>
#include <QUrl>
#include "api_client.h"

using namespace BtxSecurity;

// FIXME: See ApiResponse constructor. Horrible way to handle this.
QMap<int, const char *> TEXTSECURE_ERRORS;

/*
 * EXCEPTION
 */

ApiException::ApiException(QString msg) {
    this->msg = msg;
}

const char* ApiException::what() const throw() {
    return this->msg.toStdString().c_str();
}

ApiException::~ApiException() throw() {
    qDebug("destroy exception");
}

/*
 * RESPONSE
 */

ApiResponse::ApiResponse(QObject *parent, int resCode, QString resBody) : QObject(parent) {
    // FIXME: This needs to be something sane. Anyone who knows c++ can give a hand.
    TEXTSECURE_ERRORS[200] = NULL;
    TEXTSECURE_ERRORS[400] = "Bad number or badly formatted tokens";
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
        throw ApiException(errorMessage.toStdString().c_str());
    }

    errorMessage = TEXTSECURE_ERRORS[this->resCode];
    if (errorMessage.length()) throw ApiException(errorMessage);
}

ApiResponse::~ApiResponse() {
    qDebug("destroy response");
}

/*
 * CLIENT
 */

ApiClient::ApiClient(QString baseUrl, QObject *parent) : QObject(parent) {
    if (!baseUrl.endsWith("/")) baseUrl += "/";

    this->baseUrl = baseUrl;
}

QUrl ApiClient::genPath(QString path) {
    if (!path.startsWith("/")) path = "/" + path;

    QUrl url = baseUrl + QString("v") + QString::number(ApiClient::apiVersion) + path;
    qDebug() << url.toString() << "\n";

    return url;
}

ApiClient::~ApiClient() {
    qDebug("bye");
}

