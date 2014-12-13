// vim: ts=4 sw=4 fenc=utf-8
#include <QString>
#include "btx_exception.h"

using namespace BtxSecurity;

BtxException::BtxException(QString msg) {
    this->msg = msg;
}

const char* BtxException::what() const throw() {
    return this->msg.toStdString().c_str();
}

BtxException::~BtxException() throw() {
    qDebug("destroy exception");
}

