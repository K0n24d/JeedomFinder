#include "quickstartwizard.h"
#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QTextCodec>
#endif

#include <QMutex>
#include <QMutexLocker>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QThread>
#include <iostream>
#include <QtDebug>


QMutex logFileMutex;

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
void myMessageOutput(QtMsgType type, const char *msg)
#else
void myMessageOutput(QtMsgType type, const QMessageLogContext &, const QString &msg)
#endif
{
    QMutexLocker locker(&logFileMutex);

    static QFile logFile;

    if(!logFile.isOpen())
    {
        QString logFileName = "QuickStartGUI.log";

        QDir logDir(QDir::temp());
        if(logDir.exists())
        {
            logFileName = logDir.absoluteFilePath(logFileName);
        }

        logFile.setFileName(logFileName);
        logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    }

    QString level("Unknown");
    switch (type)
    {
    case QtDebugMsg:
        level = "Debug";
        break;
    case QtWarningMsg:
        level = "Warning";
        break;
    case QtCriticalMsg:
        level = "Critical";
        break;
    case QtFatalMsg:
        level = "Fatal";
        break;
    }

    QString fullmsg = QString("%1 %4 QuickStartGUI[%2]: %3\n")
                        .arg(QDateTime::currentDateTime().toString("MMM dd hh:mm:ss.zzz"))
                        .arg((unsigned long)QThread::currentThreadId())
                        .arg(msg)
                        .arg(level);

    if(logFile.isWritable())
    {
        logFile.write(fullmsg.toLocal8Bit());
        logFile.flush();
    }

    std::cout << fullmsg.toLocal8Bit().data();
    std::cout.flush();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    qInstallMsgHandler(myMessageOutput);
#else
    qInstallMessageHandler(myMessageOutput);
#endif


#ifndef QT_NO_TRANSLATION
    QString translatorFileName = QLatin1String("qt_");
    translatorFileName += QLocale::system().name();
    QTranslator *translator = new QTranslator(&app);
    if (translator->load(translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(translator);
#endif

    QuickStartWizard wizard;
    wizard.show();

    return app.exec();
}
