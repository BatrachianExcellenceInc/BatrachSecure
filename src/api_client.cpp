// vim: ts=4 sw=4 fenc=utf-8
#include "api_client.h"

using namespace BtxSecurity;

/*
 * CLIENT
 */

ApiClient::ApiClient(QObject *parent) : QObject(parent) {
    qDebug("construct");
}

ApiClient::~ApiClient() {
    qDebug("destruct");
}

