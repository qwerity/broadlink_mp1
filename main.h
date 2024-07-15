#pragma once

#include <QTextStream>
#include <QDir>
#include <QTime>
#include <QSettings>
#include <QDebug>

#ifdef WITHOUT_CONSOLE
#include <Windows.h>

#define MAIN()  int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#define MAIN_ARGC_ARGV \
            char** argv = __argv; \
            int argc = __argc; \
            Q_UNUSED(hInstance); \
            Q_UNUSED(hPrevInstance); \
            Q_UNUSED(lpCmdLine); \
            Q_UNUSED(nCmdShow);
#else
#define MAIN()  int main(int argc, char *argv[])
    #define MAIN_ARGC_ARGV
#endif

static void logHandler(QtMsgType type, const QMessageLogContext &logContext, const QString &msg)
{
    QString log;
    switch (type)
    {
        case QtDebugMsg:   log = QString("|Debug|%1|%2|").arg(QTime::currentTime().toString(), msg);    break;
        case QtInfoMsg:    log = QString("|Info|%1|%2|").arg(QTime::currentTime().toString(), msg);     break;
        case QtWarningMsg: log = QString("|Warning|%1|%2|").arg(QTime::currentTime().toString(), msg);  break;
        case QtCriticalMsg:log = QString("|Critical|%1|%2|").arg(QTime::currentTime().toString(), msg); break;
        case QtFatalMsg:   log = QString("|Fatal|%1|%2|").arg(QTime::currentTime().toString(), msg);    break;
    }

    static const QString sarControlLogFile = "broadlink_mp1_" + QDateTime::currentDateTime().toString("dd_MM_yyyy_hh_mm_ss_zzz") + ".log";
    QFile logFile(sarControlLogFile);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        QTextStream ts(&logFile);
        ts << log << Qt::endl;
    }
    else {
        qCritical() << "Unable to open log file";
    }

    logFile.close();
}

/**********************************************************************************************************************/