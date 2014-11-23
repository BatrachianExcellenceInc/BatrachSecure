// vim: ts=4 sw=4 fenc=utf-8
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include "api_client.h"

using namespace BtxSecurity;

const QString DEFAULT_BASE_URL("https://textsecure-service.whispersystems.org/");

enum CommandLineParseResult {
    CommandLineError,
    CommandLineOk
};

// arguments for client and its call
struct Signature {
    QStringList clientArguments;
    QStringList arguments;
};

CommandLineParseResult parseCommandLine(QCommandLineParser &parser, QCoreApplication &btxsecure, Signature *sig, QString *errorMessage) {
    // options
    const QCommandLineOption getVerificationCodeOpt(QStringList("get-verification-code"), "Get verification code");
    parser.addOption(getVerificationCodeOpt);

    const QCommandLineOption transportOpt(QStringList("transport"), "Transport for verification code. Valid: sms or voice", "transport");
    parser.addOption(transportOpt);

    const QCommandLineOption numberOpt(QStringList("number"), "Phone number for verification code. International format +358...", "number");
    parser.addOption(numberOpt);

    // positional arguments
    parser.addPositionalArgument("baseurl", QString("Base url. Default: ") + DEFAULT_BASE_URL);

    // process before access
    parser.process(btxsecure);

    // finally do something
    const QStringList args = parser.positionalArguments();

    if (args.count() == 0)
        sig->clientArguments << DEFAULT_BASE_URL;
    else
        sig->clientArguments << args.at(0);

    if (parser.isSet(getVerificationCodeOpt)) {
        if (!parser.isSet(transportOpt)) {
            *errorMessage = "Transport required";
            return CommandLineError;
        }
        if (!parser.isSet(numberOpt)) {
            *errorMessage = "Phone number required";
            return CommandLineError;
        }
        sig->arguments << parser.value(transportOpt) << parser.value(numberOpt);
    } else {
        *errorMessage = "Command required";
        return CommandLineError;
    }

    return CommandLineOk;
}

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

    Signature sig; // These get passed to whatever we call
    QString errorMessage;
    switch (parseCommandLine(parser, btxsecure, &sig, &errorMessage)) {
        case CommandLineError:
            qCritical() << errorMessage;
            qCritical() << argv[0] << "--help for more info";
            return 1;
        case CommandLineOk:
            break;
    }

    static ApiClient *client = new ApiClient(sig.clientArguments.at(0));

    delete client;
    return 0;
}

