// vim: ts=4 sw=4 fenc=utf-8
#include <QDir>
#include <QObject>
#include <stdint.h>
#include "btx_exception.h"
#include "pre_key_store.h"
#include "util/keyhelper.h"

namespace BtxSecurity {
    class ClientConfException : public BtxException {
        public:
            ClientConfException(QString msg);
    };

    class ClientConf : public QObject {
        Q_OBJECT
        public:
            // methods
            explicit ClientConf(QString, QString, QString, QObject *parent = 0);
            QString getBaseUrl();
            QString getNumber();
            QString getPassword();
            quint64 getRegistrationId();
            QByteArray getSignalingKey();
        private:
            // members
            KeyHelper keyHelper;
            BtxPreKeyStore preKeyStore;
            QString baseUrl;
            QString confDirPath;
            QDir confDir;

            QString number;
            QString password;
            quint64 registrationId;
            QByteArray signalingKey;
            IdentityKeyPair identityKeyPair;

            // methods
            void verifyConfDir();
            void verifyNumber();
            void verifyPassword();
            void verifyRegistrationId();
            void verifySignalingKey();
            void verifyIdentityKeyPair();
    };
}


