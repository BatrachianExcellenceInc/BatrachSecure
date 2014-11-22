// vim: ts=4 sw=4 fenc=utf-8
#include <QObject>

namespace BtxSecurity {
    class ApiClient : public QObject {
        Q_OBJECT;
        public:
            explicit ApiClient(QObject *parent = 0);

            ~ApiClient();
    };
}

