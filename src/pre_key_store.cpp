// vim: ts=4 sw=4 fenc=utf-8
#include <QDebug>
#include <QFile>
#include <QVector>
#include <cstdlib>
#include "pre_key_store.h"
#include "state/prekeystore.h"
#include "util/medium.h"

using namespace BtxSecurity;

/**
 * Exception
 */
PreKeyStoreException::PreKeyStoreException(QString msg) : BtxException(msg){
}

/**
 * Actual code
 */
BtxPreKeyStore::BtxPreKeyStore(QString preKeyDir, QString signedPreKeyDir, KeyHelper keyHelper) {
    this->preKeyDirPath = preKeyDir;
    this->preKeyDir = QDir(preKeyDirPath);

    this->signedPreKeyDirPath = signedPreKeyDir;
    this->signedPreKeyDir = QDir(signedPreKeyDirPath);

    this->keyHelper = keyHelper;

    this->verifyPreKeyDir();
    this->verifySignedPreKeyDir();

    this->verifyPreKeys();
    this->verifyLastResortPreKey();
}

QString BtxPreKeyStore::getKeyFile(qulonglong pre_key_id) {
    QString keyFileName(this->preKeyDirPath + QString("/") + QString::number(pre_key_id));

    return keyFileName;
}

void BtxPreKeyStore::verifyPreKeyDir() {
    if (!this->preKeyDir.exists()) {
        qDebug() << "Creating" << this->preKeyDirPath;
        if (!this->preKeyDir.mkpath(this->preKeyDirPath)) {
            throw PreKeyStoreException(QString("Could not create conf dir " + this->preKeyDirPath));
        }
    }
}

void BtxPreKeyStore::verifySignedPreKeyDir() {
    if (!this->signedPreKeyDir.exists()) {
        qDebug() << "Creating" << this->signedPreKeyDirPath;
        if (!this->signedPreKeyDir.mkpath(this->signedPreKeyDirPath)) {
            throw PreKeyStoreException(QString("Could not create conf dir " + this->signedPreKeyDirPath));
        }
    }
}

void BtxPreKeyStore::verifyPreKeys() {
    qulonglong startId = this->getNextPreKeyId();
    int count = 100;
    QList<PreKeyRecord> keys = this->keyHelper.generatePreKeys(startId, count);
    Q_ASSERT(keys.size() == count);
    for (PreKeyRecord pkr : keys) {
        qDebug() << "Storing prekey" << QString::number(pkr.getId());
        this->storePreKey(pkr.getId(), pkr);
    }
    this->setNextPreKeyId((startId + count + 1) % Medium::MAX_VALUE);
}

void BtxPreKeyStore::verifyLastResortPreKey() {
    qulonglong maxId = Medium::MAX_VALUE;

    if (this->containsPreKey(maxId)) return;

    qDebug() << "last resort maxId" << maxId;
    int count = 1;

    QList<PreKeyRecord> lastResortKeys = this->keyHelper.generatePreKeys(maxId, count);
    PreKeyRecord record = lastResortKeys.at(0);

    QFile keyFile(this->getKeyFile(maxId));

    if (!keyFile.open(QIODevice::WriteOnly)) {
        throw PreKeyStoreException(QString("Could not create last resort key file " + keyFile.fileName()));
    }

    // TODO: the encryption stuff
    this->storePreKey(maxId, record);
}

PreKeyRecord BtxPreKeyStore::loadPreKey(qulonglong preKeyId) {
    QFile keyFile(this->getKeyFile(preKeyId));

    if (!keyFile.open(QIODevice::ReadOnly)) {
        throw PreKeyStoreException(QString("Could not open key file " + keyFile.fileName()));
    }

    QDataStream keyFileStream(&keyFile);
    QByteArray data;

    keyFileStream >> data;

    PreKeyRecord record(data);

    qDebug() << "Deserialized from string" << data.size();

    return record;
}

void BtxPreKeyStore::storePreKey(qulonglong preKeyId, const PreKeyRecord &record) {
    QFile keyFile(this->getKeyFile(preKeyId));

    if (!keyFile.open(QIODevice::WriteOnly)) {
        throw PreKeyStoreException(QString("Could not create key file " + keyFile.fileName()));
    }

    // TODO: the encryption stuff
    QDataStream keyFileStream(&keyFile);

    QByteArray data;
    data = record.serialize();
    qDebug() << "Serialized to string" << data.size();

    keyFileStream << data;
}

bool BtxPreKeyStore::containsPreKey(qulonglong preKeyId) {
    return QFile(this->getKeyFile(preKeyId)).exists();
}

void BtxPreKeyStore::removePreKey(qulonglong preKeyId) {
    QFile keyFile(this->getKeyFile(preKeyId));

    if (!keyFile.open(QIODevice::WriteOnly)) {
        throw PreKeyStoreException(QString("Could not open key file " + keyFile.fileName()));
    }

    bool ok = keyFile.remove();
    if (!ok) throw PreKeyStoreException(QString("Delete failed: " + keyFile.fileName()));
}

int BtxPreKeyStore::countPreKeys() {
    QDir dir(this->preKeyDirPath + QString("/"));
    return dir.entryList().size();
}

void BtxPreKeyStore::setNextPreKeyId(qulonglong id) {
    QFile preKeyIndexFile(this->preKeyDirPath + QString("/") + "index.dat");
    if (!preKeyIndexFile.open(QIODevice::WriteOnly)) {
        throw PreKeyStoreException(QString("Could not write to key index file " + preKeyIndexFile.fileName()));
    }

    QDataStream preKeyIndexFileStream(&preKeyIndexFile);

    qDebug() << "Set pre key id" << QString::number((quint32) id);
    preKeyIndexFileStream << (quint32) id;
}

qulonglong BtxPreKeyStore::getNextPreKeyId() {
    QFile preKeyIndexFile(this->preKeyDirPath + QString("/") + "index.dat");

    if (!preKeyIndexFile.exists()) {
        // XXX: This is SecureRandom on Android, but beecrypt doesn't do .nextInt() so whatever
        srand(time(NULL));
        return rand();
    }

    if (!preKeyIndexFile.open(QIODevice::ReadOnly)) {
        throw PreKeyStoreException(QString("Could not read key index file " + preKeyIndexFile.fileName()));
    }

    QDataStream preKeyIndexFileStream(&preKeyIndexFile);
    qint32 id;
    preKeyIndexFileStream >> id;

    qDebug() << "Next pre key id" << QString::number(id);
    return (qulonglong) id;
}

