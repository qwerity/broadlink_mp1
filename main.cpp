#include "main.h"
#include "mainwindow.h"

#include <QApplication>
#include <QCryptographicHash>

MAIN()
{
    MAIN_ARGC_ARGV;

    QApplication a(argc, argv);

    static QString lockFileName = QDir::tempPath() + QDir::separator();
    lockFileName += QCryptographicHash::hash("broadlink_mp1", QCryptographicHash::RealSha3_224).toHex();
    lockFileName += ".lock";
    static QLockFile lockFile(lockFileName);
    if (! lockFile.tryLock(100))
    {
        exit(0);
    }

    qInstallMessageHandler(logHandler);

    MainWindow w;
    w.show();

    return a.exec();
}
