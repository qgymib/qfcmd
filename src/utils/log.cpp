#include <cstdio>
#include <QtGlobal>
#include "log.hpp"

namespace qfcmd {
struct LogCtx
{
    LogCtx(const QString& path);
    ~LogCtx();

    FILE* logfile;
};
} /* namespace qfcmd */

static qfcmd::LogCtx* s_log = nullptr;

qfcmd::LogCtx::LogCtx(const QString& path)
{
    if (path.isEmpty())
    {
        logfile = nullptr;
        return;
    }

    std::string log_path = path.toStdString();
    logfile = fopen(log_path.c_str(), "w");
}

qfcmd::LogCtx::~LogCtx()
{
    if (logfile != nullptr)
    {
        fclose(logfile);
    }
    logfile = nullptr;
}

static void _log_msg_handler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg)
{
    QString message = qFormatLogMessage(type, ctx, msg);

    FILE* logfile = stderr;
    if (s_log != nullptr && s_log->logfile != nullptr)
    {
        logfile = s_log->logfile;
    }

    fprintf(logfile, "%s", qUtf8Printable(message));
    fflush(logfile);
}

void qfcmd::Log::init(const QString& path)
{
    if (s_log != nullptr)
    {
        return;
    }

    s_log = new qfcmd::LogCtx(path);

    qSetMessagePattern("[%{time yyyyMMdd h:mm:ss.zzz t} %{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] %{file}:%{line} - %{message}");
    qInstallMessageHandler(_log_msg_handler);
}

void qfcmd::Log::exit()
{
    if (s_log == nullptr)
    {
        return;
    }

    qInstallMessageHandler(nullptr);
    delete s_log;
    s_log = nullptr;
}
