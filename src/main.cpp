// vim: ts=4 sw=4 fenc=utf-8
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMetaObject>
#include <QString>
#include "api_client.h"

using namespace BtxSecurity;

const QString DEFAULT_BASE_URL("https://textsecure-service.whispersystems.org/");
const QString DEFAULT_CONF_DIR(".btxsec/");

enum CommandLineParseResult {
    CommandLineError,
    CommandLineOk
};

// This is explained in main()
enum ClientMethods {
    IGNORE,
    GET_VERIFICATION_CODE,
    CONFIRM_VERIFICATION_CODE,
};

// arguments for client and its call
struct Signature {
    QString confDir;
    QString number;
    QString password;

    QStringList clientArguments;
    int methId;
    QStringList arguments;
};

/**
 * Parse command line options
 */
CommandLineParseResult parseCommandLine(QCommandLineParser &parser, QCoreApplication &btxsecure, Signature *sig, QString *errorMessage) {
    /*
     * Some options specific to us
     */

    // confdir
    const QCommandLineOption confDirOpt(QStringList("conf-dir"), QString("Conf dir under home. Defaults to " + DEFAULT_CONF_DIR), "conf_dir", DEFAULT_CONF_DIR);
    parser.addOption(confDirOpt);

    // setup number and password into confdir
    const QCommandLineOption writeConfOpt(QStringList("write-conf"), "Write configuration file, requires --number and --password");
    parser.addOption(writeConfOpt);
    const QCommandLineOption numberOpt(QStringList("number"), "Phone number for verification code. International format +358...", "number");
    parser.addOption(numberOpt);

    /*
     * Actions
     */
    const QCommandLineOption getVerificationCodeOpt(QStringList("get-verification-code"), "Get verification code");
    parser.addOption(getVerificationCodeOpt);

    const QCommandLineOption transportOpt(QStringList("transport"), "Transport for verification code. Valid: sms or voice", "transport");
    parser.addOption(transportOpt);

    const QCommandLineOption confirmVerificationCodeOpt(QStringList("confirm-verification-code"), "Confirm verification code", "code");
    parser.addOption(confirmVerificationCodeOpt);

    // positional arguments
    parser.addPositionalArgument("baseurl", QString("Base url. Default: ") + DEFAULT_BASE_URL, DEFAULT_BASE_URL);

    // process before access
    parser.process(btxsecure);

    // finally do something
    const QStringList args = parser.positionalArguments();

    // Always set the host
    sig->clientArguments << args.at(0);

    // Always set the path
    sig->confDir = parser.value(confDirOpt);
    QString confPathName = QDir::homePath() + "/" + sig->confDir;
    QDir confPath(confPathName);
    if (!confPath.exists()) confPath.mkpath(confPath.path());

    // Do we configure?
    QFile numberFile(confPathName + QString("/number.conf"));
    QFile passwordFile(confPathName + QString("/password.conf"));
    if (parser.isSet(writeConfOpt)) {
        if (!parser.isSet(numberOpt)) {
            *errorMessage = "Phone number required";
            return CommandLineError;
        }

        // Write the number so we can use ClientConf and ignore any real execution
        QDataStream nfWriter(&numberFile);

        numberFile.open(QFile::WriteOnly);
        nfWriter << parser.value(numberOpt);
        numberFile.close();

        sig->number = parser.value(numberOpt);
        sig->methId = ClientMethods::IGNORE;
    } else if (!numberFile.exists() || !passwordFile.exists()) {
        sig->methId = ClientMethods::IGNORE;
        *errorMessage = "Run with --write-conf first";
        return CommandLineError;
    } else {
        QDataStream nfReader(&numberFile);

        numberFile.open(QFile::ReadOnly);
        nfReader >> sig->number;
        numberFile.close();
    }

    // Do we run real commands?
    if (parser.isSet(getVerificationCodeOpt)) {
        if (!parser.isSet(transportOpt)) {
            *errorMessage = "Transport required";
            return CommandLineError;
        }
        sig->methId = ClientMethods::GET_VERIFICATION_CODE;
        sig->arguments << parser.value(transportOpt);
    } else if (parser.isSet(confirmVerificationCodeOpt)) {
        sig->methId = ClientMethods::CONFIRM_VERIFICATION_CODE;
        sig->arguments << parser.value(confirmVerificationCodeOpt);
    } else if (sig->methId != ClientMethods::IGNORE) {
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

    QString baseUrl = sig.clientArguments.at(0);
    static ClientConf *conf = new ClientConf(baseUrl, sig.confDir, sig.number);
    static ApiClient *client = new ApiClient(conf);

    // Passing the method name like QString.toStdString().c_str() will get
    // qt5 stuck waiting for godot.
    // Assigning that to a variable and then calling also gets stuck.
    bool exitImmediately = false;
    switch (sig.methId) {
        case ClientMethods::IGNORE:
            exitImmediately = true;
            break;
        case ClientMethods::GET_VERIFICATION_CODE:
            QMetaObject::invokeMethod(client, "getVerificationCode", Qt::QueuedConnection,
                Q_ARG(QString, sig.arguments.at(0)));
            break;
        case ClientMethods::CONFIRM_VERIFICATION_CODE:
            Q_ASSERT(sig.arguments.at(0) != QString(""));
            QMetaObject::invokeMethod(client, "confirmVerificationCode", Qt::QueuedConnection,
                Q_ARG(QString, sig.arguments.at(0)));
            break;
        default:
            qDebug() << "Unknown method id" << sig.methId;
            exitImmediately = true;
    }

    // Stay in the event loop to keep the client available until the action is performed
    int retval = 1;
    if (!exitImmediately) {
        retval = btxsecure.exec();
    }

    delete client;
    return retval;
}

