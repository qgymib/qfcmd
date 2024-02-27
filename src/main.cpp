#include "mainwindow.hpp"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QCommandLineParser>
#include <QSettings>
#include <QStandardPaths>

static void _setup_i18n(QApplication& a)
{
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "qfcmd_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
}

static void _setup_app(QApplication& a)
{
    const QString organization_domain = "com.github.qgymib.qfcmd";
    QApplication::setOrganizationName(organization_domain);
    QApplication::setOrganizationDomain(organization_domain);
    QApplication::setApplicationName("qfcmd");

    QCommandLineParser parser;
    parser.setApplicationDescription(QApplication::translate("MainWindow", "A quick and powerfull file manager."));

    const QCommandLineOption opt_help = parser.addHelpOption();
    const QCommandLineOption opt_version = parser.addVersionOption();

    const QCommandLineOption opt_config("config",
                                        QApplication::translate("MainWindow", "Configuration directory."),
                                        "directory",
                                        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    parser.addOption(opt_config);

    if (!parser.parse(QApplication::arguments()))
    {
        QTextStream(stderr) << parser.errorText() << Qt::endl;
        exit(EXIT_FAILURE);
    }

    if (parser.isSet(opt_version))
    {
        parser.showVersion();
        Q_UNREACHABLE_RETURN();
    }

    if (parser.isSet(opt_help))
    {
        parser.showHelp();
        Q_UNREACHABLE_RETURN();
    }

    /* Set config file format and location. */
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QApplication::applicationDirPath());
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    _setup_i18n(a);
    _setup_app(a);

    MainWindow w;
    w.show();

    return a.exec();
}
