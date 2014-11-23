// vim: ts=4 sw=4 fenc=utf-8
#include <QNetworkReply>
#include <QObject>
#include <QUrl>
#include <exception>

namespace BtxSecurity {
    class ApiException : public std::exception {
        private:
            QString msg;
        public:
            ApiException(QString msg);
            virtual const char* what() const throw();
            ~ApiException() throw();
    };

    class ApiResponse : public QObject {
        Q_OBJECT;
        public:
            // members
            int resCode;
            QString resBody;

            // methods
            explicit ApiResponse(QObject *parent = 0, int resCode=0, QString resBody="(empty)");
            void validate();
            ~ApiResponse();
    };

    class ApiClient : public QObject {
        Q_OBJECT;
        public:
            explicit ApiClient(QString baseUrl, QObject *parent = 0);
            ~ApiClient();
        protected:
            // members
            QString baseUrl;
            QNetworkAccessManager mgr;
            static const int apiVersion = 1;

            // methods
            QUrl genPath(QString);
            void getResponse(QUrl);
            void handleNetworkResponse(QNetworkReply *);
            void handleResponse(ApiResponse *);
            void printSslErrors(QList<QSslError>);
        public slots:
            void getVerificationCode(QString, QString);
    };
}

