// vim: ts=4 sw=4 fenc=utf-8
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include "api_client.h"

using namespace BtxSecurity;

const QString DEFAULT_BASE_URL("https://textsecure-service.whispersystems.org/");

int main(int argc, char *argv[]) {
    // basic construction
    QCoreApplication btxsecure(argc, argv);
    QCoreApplication::setApplicationName("btxsecure");
    QCoreApplication::setApplicationVersion("0.1");

    // what's needed to do anything
    QCommandLineParser parser;
    parser.setApplicationDescription("By the grace of the Great Toad, a CLI app for TextSecure.");
    parser.addHelpOption();
    parser.addVersionOption();

    // positional arguments
    parser.addPositionalArgument("baseurl", "Base url");

    // process before access
    parser.process(btxsecure);

    // finally do something
    const QStringList args = parser.positionalArguments();

    QString baseUrl;
    if (args.count() == 0)
        baseUrl = DEFAULT_BASE_URL;
    else
        baseUrl = args.at(0);

    Q_ASSERT(baseUrl != NULL);
    Q_ASSERT(baseUrl.startsWith("http"));

    static ApiClient *client = new ApiClient(baseUrl);

    delete client;
    return 0;
}

