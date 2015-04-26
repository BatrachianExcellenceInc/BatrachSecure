// vim: ts=4 sw=4 fenc=utf-8
#pragma once

#include <QDir>
#include <QString>
#include "btx_exception.h"
#include "state/prekeystore.h"
#include "util/keyhelper.h"

namespace BtxSecurity {
    class PreKeyStoreException : public BtxException {
        public:
            PreKeyStoreException(QString msg);
    };

    class BtxPreKeyStore : public PreKeyStore {
        private:
            QString getKeyFile(qulonglong pre_key_id);
            QString preKeyDirPath;
            QDir preKeyDir;
            QString signedPreKeyDirPath;
            QDir signedPreKeyDir;
            QDir confDir;
            KeyHelper keyHelper;
        public:
            BtxPreKeyStore() {};  // Keep the compiler happy
            BtxPreKeyStore(QString preKeyDir, QString signedPreKeyDir, KeyHelper keyHelper);
            void verifyPreKeyDir();
            void verifySignedPreKeyDir();
            void verifyPreKeys();
            void verifyLastResortPreKey();

            PreKeyRecord loadPreKey(qulonglong preKeyId);
            void storePreKey(qulonglong preKeyId, const PreKeyRecord &record);
            bool containsPreKey(qulonglong preKeyId);
            void removePreKey(qulonglong preKeyId);
            int countPreKeys();

            void setNextPreKeyId(qulonglong id);
            qulonglong getNextPreKeyId();
    };
}

