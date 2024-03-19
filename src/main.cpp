#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QCommandLineParser>
#include <QSettings>
#include <QStandardPaths>

#if defined(_WIN32)
#include <windows.h>
#endif

#include "qfcmd/qfcmd.h"
#include "vfs/vfs.hpp"
#include "vfs/virtual.hpp"
#include "widget/mainwindow.hpp"
#include "utils/log.hpp"
#include "settings.hpp"

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
    (void)a;

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

    const QCommandLineOption opt_log("log",
    QApplication::translate("MainWindow", "Log file path."),
                                        "path");
    parser.addOption(opt_log);

    if (!parser.parse(QApplication::arguments()))
    {
        QTextStream(stderr) << parser.errorText() << Qt::endl;
        exit(EXIT_FAILURE);
    }

    if (parser.isSet(opt_version))
    {
        parser.showVersion();
        Q_UNREACHABLE();
    }

    if (parser.isSet(opt_help))
    {
        parser.showHelp();
        Q_UNREACHABLE();
    }

    QString logfile = "./qfcmd.log";
    if (parser.isSet(opt_log))
    {
        logfile = parser.value(opt_log);
    }
    qfcmd::Log::init(logfile);
    qfcmd::Settings::init();
    qfcmd::VFS::init();
    qfcmd::VirtualFS::init();
}

/**
 * @brief Global exit hook.
 */
static void _at_exit()
{
    qfcmd::VirtualFS::exit();
    qfcmd::VFS::exit();
    qfcmd::Settings::exit();
    qfcmd::Log::exit();
}

int main(int argc, char *argv[])
{
    /*
     * Register as soon as possible.
     * @see https://en.cppreference.com/w/cpp/utility/program/atexit
     */
    std::atexit(_at_exit);

#if defined(_WIN32)
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
#endif

    QApplication a(argc, argv);
    _setup_i18n(a);
    _setup_app(a);

    qfcmd::MainWindow w;
    w.show();

    return a.exec();
}
