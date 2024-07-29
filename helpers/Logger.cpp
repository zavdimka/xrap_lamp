#include "Logger.h"
#include <QString>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(_log, "log")

/// Define default handler pattern.
const QString pattern = "[%{time dd.MM.yyyy h:mm:ss.zzz} %{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif} %{category}] %{file}:%{line}:%{function} - %{message}";
/// Get the default Qt message handler.
const QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER = qInstallMessageHandler(0);

/// Name for log file.
QString logFilePath;
QFile logFile;
bool logFileIsOpen;

/// Create/find log directory and init logFile path.
void CreateLogFilePath(QString projectName)
{
    /// Find project dir.
    QDir dir;
    // while (dir.dirName() != projectName)
    //     dir.cdUp();

    /// Create/find subdir for logs.
    QString logDir = "logs";
    if (!dir.exists(logDir))
        dir.mkdir(logDir);
    dir.cd(logDir);

    /// Create logFile path and init it.
    QString addPath = QString("/log_%1.txt").arg(QDateTime::currentDateTime().toString("ddMMyyyy_hhmmss"));
    logFilePath = dir.absolutePath() + addPath;
    logFile.setFileName(logFilePath);
}

/// Custom message handler, addes output to file.
void customMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    /// Call the default handler.
    (*QT_DEFAULT_MESSAGE_HANDLER)(type, context, msg);

    /// Log to file.
    if (logFileIsOpen) {
        logFile.write(qFormatLogMessage(type, context, msg).toUtf8() + '\n');
        logFile.flush();
    }
}

void InitLogger(QString projectName)
{
    qSetMessagePattern(pattern);

    /// Open log file.
    CreateLogFilePath(projectName);
    logFileIsOpen = logFile.open(QIODevice::Append | QIODevice::Text);
    if (logFileIsOpen)
        qCInfo(_log).noquote() << QString("Log file %1 was opened.").arg(logFile.fileName());
    else
        qCWarning(_log) << "Log file open error.";

    /// Install custom handler.
    qInstallMessageHandler(customMessageOutput);
    qSetMessagePattern(pattern);
}

