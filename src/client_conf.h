// vim: ts=4 sw=4 fenc=utf-8
#include <QDir>
#include <QObject>
#include <stdint.h>
#include "btx_exception.h"
#include "keyhelper.h"

namespace BtxSecurity {
    class ClientConfException : public BtxException {
        public:
            ClientConfException(QString msg);
    };

    class ClientConf : public QObject {
        Q_OBJECT
        public:
            // methods
            explicit ClientConf(QString confDir, QObject *parent = 0);
        private:
            // members
            KeyHelper keyHelper;
            QString confDirPath;
            QDir confDir;

            uint16_t registrationId;
            ECKeyPair identityKeyPair;

            // methods
            void verifyConfDir();
            void verifyRegistrationId();
            void verifyIdentityKeyPair();
    };
}

