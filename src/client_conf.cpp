// vim: ts=4 sw=4 fenc=utf-8
#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QVector>
#include <stdlib.h>
#include "client_conf.h"
#include "ecc/eckeypair.h"
#include "util/keyhelper.h"

using namespace BtxSecurity;

/*
 * EXCEPTION
 */

ClientConfException::ClientConfException(QString msg) : BtxException(msg){
}

/*
 * CLIENT CONF
 */

ClientConf::ClientConf(QString baseUrl, QString confDir, QString number, QObject *parent) : QObject(parent) {
    // Some basics are always required
    if (!baseUrl.endsWith("/")) baseUrl += "/";
    this->baseUrl = baseUrl;

    // XXX: Will not work with other separator
    this->confDirPath = QDir::homePath() + "/" + confDir;
    this->confDir = QDir(this->confDirPath);

    qDebug() << QString("ClientConf constructed with number " + number);
    this->number = number;

    this->verifyConfDir();
    this->verifyNumber();
    this->verifyPassword();
    this->verifyRegistrationId();
    this->verifySignalingKey();
    this->verifyIdentityKeyPair();

    this->preKeyStore = BtxPreKeyStore(this->confDirPath + QString("/prekeys"), this->confDirPath + QString("/signed_prekeys"), this->keyHelper);
}

QString ClientConf::getBaseUrl() {
    return this->baseUrl;
}

QString ClientConf::getNumber() {
    qDebug() << QString("getNumber() returning ") + this->number;
    return this->number;
}

QString ClientConf::getPassword() {
    return this->password;
}

quint64 ClientConf::getRegistrationId() {
    return this->registrationId;
}

QByteArray ClientConf::getSignalingKey() {
    return this->signalingKey;
}

void ClientConf::verifyConfDir() {
    if (!this->confDir.exists()) {
        qDebug() << "Creating" << this->confDirPath;
        if (!this->confDir.mkpath(this->confDirPath)) {
            throw ClientConfException(QString("Could not create conf dir " + this->confDirPath));
        }
    }
}

void ClientConf::verifyNumber() {
    QFile numberFile(this->confDirPath + QString("/number.conf"));
    if (!numberFile.exists()) {
        if (!numberFile.open(QIODevice::WriteOnly)) {
            throw ClientConfException(QString("Could not create phone number file " + numberFile.fileName()));
        }

        QDataStream numberFileDS(&numberFile);
        numberFile.open(QFile::ReadOnly);
        numberFileDS << this->number;
        numberFile.close();
    }
}

void ClientConf::verifyPassword() {
    QFile passwordFile(this->confDirPath + QString("/password.conf"));
    if (!passwordFile.exists()) {
        if (!passwordFile.open(QIODevice::WriteOnly)) {
            throw ClientConfException(QString("Could not create random password file " + passwordFile.fileName()));
        }

        QByteArray pwChars = this->keyHelper.getRandomBytes(16);
        QString pw(pwChars);

        QDataStream pwWriter(&passwordFile);
        pwWriter << pw;
        passwordFile.close();

        this->password = pw;
    } else {
        QDataStream pwReader(&passwordFile);
        passwordFile.open(QFile::ReadOnly);
        pwReader >> this->password;
        passwordFile.close();
    }
}

void ClientConf::verifyRegistrationId() {
    QString regIdFilePath = QString(this->confDirPath + QString("/registration.id"));
    QFile regIdFile(regIdFilePath);

    qDebug() << regIdFilePath;
    if (!regIdFile.exists()) {
        this->registrationId = this->keyHelper.generateRegistrationId();

        qDebug() << "Generated registration id" << this->registrationId;

        if (!regIdFile.open(QIODevice::WriteOnly)) {
            throw ClientConfException(QString("Could not create registration id file " + regIdFilePath));
        }

        // Because we want to write easily
        QDataStream regIdFileStream(&regIdFile);
        regIdFileStream << this->registrationId << "\r\n";
    } else {
        if (!regIdFile.open(QIODevice::ReadOnly)) {
            throw ClientConfException(QString("Could not open registration id file " + regIdFilePath));
        }

        // Read similarly
        QDataStream regIdFileStream(&regIdFile);
        regIdFileStream >> this->registrationId;

        qDebug() << "Read registration id" << this->registrationId;
    }
}

void ClientConf::verifyIdentityKeyPair() {
    QString ikpFilePath = QString(this->confDirPath + QString("/identity.key"));
    QFile ikpFile(ikpFilePath);

    qDebug() << ikpFilePath;
    if (!ikpFile.exists()) {
        this->identityKeyPair = this->keyHelper.generateIdentityKeyPair();

        if (!ikpFile.open(QIODevice::WriteOnly)) {
            throw ClientConfException(QString("Could not create identity key file " + ikpFilePath));
        }

        // Get private key from generated pair
        QByteArray privateKey = this->identityKeyPair.getPrivateKey().serialize();

        // Get public key from generated pair
        QByteArray publicKey = this->identityKeyPair.getPublicKey().serialize();

        qDebug() << "Generated identity" << privateKey.size() << "|" << publicKey.size();

        // Write file
        QDataStream ikpFileStream(&ikpFile);
        ikpFileStream << privateKey << publicKey;
    } else {
        QByteArray privateKey;
        QByteArray publicKey;

        if (!ikpFile.open(QIODevice::ReadOnly)) {
            throw ClientConfException(QString("Could not open identity key file " + ikpFilePath));
        }

        // Read from file
        QDataStream ikpFileStream(&ikpFile);
        ikpFileStream >> privateKey >> publicKey;

        qDebug() << "Read identity" << privateKey.size() << "|" << publicKey.size();

        IdentityKey ikPublic(publicKey);
        DjbECPrivateKey ecPrivate(privateKey);

        this->identityKeyPair = IdentityKeyPair(ikPublic, ecPrivate);
    }
}

void ClientConf::verifySignalingKey() {
    QString signalKeyFilePath = QString(this->confDirPath + QString("/signaling.key"));
    QFile signalKeyFile(signalKeyFilePath);

    qDebug() << signalKeyFilePath;
    if (!signalKeyFile.exists()) {
        this->signalingKey = this->keyHelper.generateSignalingKey();

        qDebug() << "Generated signaling key" << this->signalingKey;

        if (!signalKeyFile.open(QIODevice::WriteOnly)) {
            throw ClientConfException(QString("Could not create signaling key file " + signalKeyFilePath));
        }

        // Because we want to write easily
        QDataStream signalKeyFileStream(&signalKeyFile);
        signalKeyFileStream << this->signalingKey << "\r\n";
    } else {
        if (!signalKeyFile.open(QIODevice::ReadOnly)) {
            throw ClientConfException(QString("Could not open signaling key file " + signalKeyFilePath));
        }

        // Read similarly
        QDataStream signalKeyFileStream(&signalKeyFile);
        signalKeyFileStream >> this->signalingKey;

        qDebug() << "Read signaling key" << this->signalingKey.size();
    }
}

