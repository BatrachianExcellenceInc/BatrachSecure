// vim: ts=4 sw=4 fenc=utf-8
#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QVector>
#include <stdlib.h>
#include "client_conf.h"
#include "eckeys.h"
#include "keyhelper.h"

using namespace BtxSecurity;

/*
 * EXCEPTION
 */

ClientConfException::ClientConfException(QString msg) : BtxException(msg){
}

/*
 * CLIENT CONF
 */

ClientConf::ClientConf(QString confDir, QObject *parent) : QObject(parent) {
    // XXX: Will not work with other separator
    this->confDirPath = QDir::homePath() + "/" + confDir;
    this->confDir = QDir(this->confDirPath);

    this->verifyConfDir();
    this->verifyRegistrationId();
    this->verifyIdentityKeyPair();
}

void ClientConf::verifyConfDir() {
    if (!this->confDir.exists()) {
        qDebug() << "Creating" << this->confDirPath;
        if (!this->confDir.mkpath(this->confDirPath))
            throw ClientConfException(QString("Could not create conf dir " + this->confDirPath));
    }
}

void ClientConf::verifyRegistrationId() {
    QString regIdFilePath = QString(this->confDirPath + QString("/registration.id"));
    QFile regIdFile(regIdFilePath);

    qDebug() << regIdFilePath;
    if (!regIdFile.exists()) {
        this->registrationId = this->keyHelper.generateRegistrationId();

        qDebug() << "Generated registration id" << this->registrationId;

        if (!regIdFile.open(QIODevice::WriteOnly))
            throw ClientConfException(QString("Could not create registration id file " + regIdFilePath));

        // Because we want to write easily
        QDataStream regIdFileStream(&regIdFile);
        regIdFileStream << this->registrationId << "\r\n";
        Q_ASSERT((this->registrationId & 0x3fff) == this->registrationId);
        Q_ASSERT((this->registrationId & ~0x3fff) == 0);
    } else {
        if (!regIdFile.open(QIODevice::ReadOnly))
            throw ClientConfException(QString("Could not open registration id file " + regIdFilePath));

        // Read similarly
        QDataStream regIdFileStream(&regIdFile);
        regIdFileStream >> this->registrationId;

        qDebug() << "Read registration id" << this->registrationId;
        Q_ASSERT((this->registrationId & 0x3fff) == this->registrationId);
        Q_ASSERT((this->registrationId & ~0x3fff) == 0);
    }
}

void ClientConf::verifyIdentityKeyPair() {
    QString ikpFilePath = QString(this->confDirPath + QString("/identity.key"));
    QFile ikpFile(ikpFilePath);

    qDebug() << ikpFilePath;
    if (!ikpFile.exists()) {
        this->identityKeyPair = this->keyHelper.generateIdentityKeyPair();

        if (!ikpFile.open(QIODevice::WriteOnly))
            throw ClientConfException(QString("Could not create identity key file " + ikpFilePath));

        // Get private key from generated pair
        std::vector<unsigned char> stdPrivateKey = this->identityKeyPair.getPrivateKey().serialize();
        Q_ASSERT_X(stdPrivateKey.size() == 32, "verifyIdentityKeyPair private", QString::number(stdPrivateKey.size()).toStdString().c_str());
        QVector<unsigned char> privateKey = QVector<unsigned char>::fromStdVector(stdPrivateKey);
        Q_ASSERT_X(privateKey.size() == 32, "verifyIdentityKeyPair private", QString::number(privateKey.size()).toStdString().c_str());

        // Get public key from generated pair
        std::vector<unsigned char> stdPublicKey = this->identityKeyPair.getPublicKey().serialize();
        stdPublicKey.erase(stdPublicKey.begin()); // The serialized type is not required
        Q_ASSERT_X(stdPublicKey.size() == 32, "verifyIdentityKeyPair public", QString::number(stdPublicKey.size()).toStdString().c_str());
        QVector<unsigned char> publicKey = QVector<unsigned char>::fromStdVector(stdPublicKey);
        Q_ASSERT_X(publicKey.size() == 32, "verifyIdentityKeyPair public", QString::number(publicKey.size()).toStdString().c_str());

        qDebug() << "Generated identity" << privateKey.size() << "|" << publicKey.size();

        // Write file
        QDataStream ikpFileStream(&ikpFile);
        ikpFileStream << privateKey << publicKey;
    } else {
        QVector<unsigned char> privateKey;
        QVector<unsigned char> publicKey;

        if (!ikpFile.open(QIODevice::ReadOnly))
            throw ClientConfException(QString("Could not open identity key file " + ikpFilePath));

        // Read from file
        QDataStream ikpFileStream(&ikpFile);
        ikpFileStream >> privateKey >> publicKey;

        qDebug() << "Read identity" << privateKey.size() << "|" << publicKey.size();

        ECPublicKey ecPublicKey(publicKey.toStdVector());
        ECPrivateKey ecPrivateKey(privateKey.toStdVector());

        this->identityKeyPair = ECKeyPair(ecPublicKey, ecPrivateKey);
    }
}

