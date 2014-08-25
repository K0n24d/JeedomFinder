#include "pingsearchworker.h"
#include <QProcess>
#include <QNetworkInterface>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QHostInfo>

PingSearchWorker::PingSearchWorker(QObject *parent) :
    SearchWorker(parent), stopping(false)
{
    checkResultsTimer.setInterval(1000);
    checkResultsTimer.setSingleShot(true);

    connect(&checkResultsTimer, SIGNAL(timeout()), this, SLOT(checkResults()));
}

PingSearchWorker::~PingSearchWorker()
{
    foreach(QProcess *process, pingProcesses)
    {
        if(process->state()!=QProcess::NotRunning)
            process->kill();
        process->deleteLater();
    }
}

void PingSearchWorker::discover()
{
    QStringList arguments;

    checkResultsTimer.start();

    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
    {
        if(   !interface.flags().testFlag(QNetworkInterface::IsUp)
           || interface.flags().testFlag(QNetworkInterface::IsLoopBack)
           || interface.flags().testFlag(QNetworkInterface::IsPointToPoint)
              )
            continue;

        foreach(QNetworkAddressEntry addressEntry, interface.addressEntries())
        {
            if(addressEntry.ip().protocol()!=QAbstractSocket::IPv4Protocol)
                continue;

            quint32 myaddress = addressEntry.ip().toIPv4Address();
            quint32 netmask = addressEntry.netmask().toIPv4Address();
            quint32 network = myaddress & netmask;
            quint32 address = network + 1;

            while((address & netmask) == network)
            {
                QHostAddress hostaddress(address);

                while(pingProcesses.count()>50)
                {
                    foreach(QProcess *process, pingProcesses)
                    {
                        if(stopping)
                            return;

                        if(process->state()==QProcess::NotRunning)
                        {
                            process->deleteLater();
                            pingProcesses.removeOne(process);
                        }
                    }

                    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                }

                QProcess *process = new QProcess(this);
                pingProcesses << process;
                arguments.clear();
#if defined(Q_OS_UNIX)
                arguments << "-c1" << "-n" <<"-q" << hostaddress.toString();
#elif defined(Q_OS_WIN)
                arguments << "-n" << "1" << hostaddress.toString();
#endif
                process->start("ping", arguments);
                if(!process->waitForStarted())
                {
                    emit(error(Q_FUNC_INFO, tr("Impossible de lancer ping %1").arg(process->errorString())));
                }

                address++;
            }
        }
    }

    while(pingProcesses.count()>0)
    {
        foreach(QProcess *process, pingProcesses)
        {
            if(stopping)
                return;

            if(process->state()==QProcess::NotRunning)
            {
                process->deleteLater();
                pingProcesses.removeOne(process);
            }
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    emit(finished());
}

void PingSearchWorker::stop()
{
    checkResultsTimer.stop();
    stopping = true;
}

#ifdef Q_OS_WIN
void PingSearchWorker::checkResults()
{
    if(arpTableProcess.state()!=QProcess::NotRunning)
    {
        if(numberOfSearchWorkersRunning>0)
            checkResultsTimer.start();
        return;
    }

    if(!arpTableProcess.start("arp", QStringList() << "-a"))
        emit(error(Q_FUNC_INFO, tr("Impossible de lancer l'utilitaire arp.")));
}

void PingSearchWorker::gotArpResults(int)
{
    QRegExp rx("^ *([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}) *([^ -]+-[^ -]+-[^ -]+-[^ -]+-[^ -]+-[^ -]+).*$");
            /*
    QRegExp rx(" *\\([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\)"
               " +\\([^ ]+\\)"
               " .*");
                       */
    while(!arpTableProcess.atEnd())
    {
        QString line(arpTableProcess.readLine());
        int pos = rx.indexIn(line);
        if(pos<0)
            continue;

        QStringList list = rx.capturedTexts();
        if(list.count()!=3)
            continue;

//        QString mac = list.at(2).split('-').join(':').toUpper();
        QString mac = list.at(2).split("-").join(":").toUpper();
        if(mac.startsWith("B8:27:EB") || mac.startsWith("52:54:00"))
        {
            emit(hostFound(list.at(1), mac));
        }
//        QHostInfo hostInfo(QHostInfo::fromName(address));
//        QString name = hostInfo.hostName();
    }

    checkResultsTimer.start();
}
#endif

#ifdef Q_OS_LINUX
void PingSearchWorker::checkResults()
{
    QFile arpTable("/proc/net/arp");

    if(arpTable.open(QFile::ReadOnly))
    {
        QRegExp rx("^ *([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})[^:]*([^ :]+:[^ :]+:[^ :]+:[^ :]+:[^ :]+:[^ :]+).*$");
        QByteArray data = arpTable.readAll();
        QTextStream in(&data, QIODevice::ReadOnly);

        while(!in.atEnd())
        {
            QString line(in.readLine());
            int pos = rx.indexIn(line);

            if(pos<0)
                continue;

            QStringList list = rx.capturedTexts();
            if(list.count()!=3)
                continue;

            QString mac = list.at(2).toUpper();
            if(!mac.compare("00:00:00:00:00:00"))
                continue;

            if(mac.startsWith("B8:27:EB") || mac.startsWith("52:54:00") || mac.startsWith("42:98:42"))
            {
                QHostInfo hostInfo;
                hostInfo.fromName(list.at(1));

                Host thisHost;
                thisHost.name = hostInfo.hostName();
                thisHost.ip = list.at(1);
                thisHost.desc = tr("MAC : %1").arg(mac);
                thisHost.url = QString("http://%1/jeedom").arg(thisHost.ip);
                emit(host(thisHost));
            }
        }
    }
    else
        emit(error(Q_FUNC_INFO, tr("Impossible d'ouvrir /proc/net/arp: %1").arg(arpTable.errorString())));

    if(!stopping)
        checkResultsTimer.start();
}
#endif
