// vim: ts=4 sw=4 fenc=utf-8
#include <QObject>
#include <QUrl>

namespace BtxSecurity {
    class ApiClient : public QObject {
        Q_OBJECT;
        public:
            explicit ApiClient(QString baseUrl, QObject *parent = 0);

            ~ApiClient();
        protected:
            // members
            QString baseUrl;
            static const int apiVersion = 1;

            // methods
            QUrl genPath(QString);
    };
}

