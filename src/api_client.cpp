// vim: ts=4 sw=4 fenc=utf-8
#include <QDebug>
#include <QString>
#include <QUrl>
#include "api_client.h"

using namespace BtxSecurity;

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

