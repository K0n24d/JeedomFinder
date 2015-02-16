#include "pingsearchworker.h"
#include <QProcess>
#include <QNetworkInterface>
#include <QNetworkAccessManager>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QHostInfo>
#include <QNetworkReply>
#include <QtDebug>
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
#include <QProcess>
#include <QMutexLocker>
#endif

PingSearchWorker::PingSearchWorker(QObject *parent) :
    SearchWorker(parent)
{
    qDebug() << Q_FUNC_INFO;

    checkResultsTimer=NULL;
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    arpTableProcess=NULL;
#endif
    manager=NULL;
}

PingSearchWorker::~PingSearchWorker()
{
    if(checkResultsTimer)
    {
        checkResultsTimer->stop();
        checkResultsTimer->deleteLater();
    }

    foreach(QProcess *process, pingProcesses)
    {
        if(process->state()!=QProcess::NotRunning)
            process->kill();
        process->deleteLater();
    }
}

void PingSearchWorker::discover()
{
    qDebug() << Q_FUNC_INFO;

    allRequestsSent=false;

    QStringList arguments;

    checkedMACs.clear();

    if(checkResultsTimer)
    {
        checkResultsTimer->stop();
        checkResultsTimer->deleteLater();
    }

    checkResultsTimer = new QTimer(this);
    checkResultsTimer->setInterval(1000);
    checkResultsTimer->setSingleShot(true);
    connect(checkResultsTimer, SIGNAL(timeout()), this, SLOT(checkResults()));
    checkResultsTimer->start();

    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        if(stopping)
            return;

        if(   !interface.flags().testFlag(QNetworkInterface::IsUp)
           || interface.flags().testFlag(QNetworkInterface::IsLoopBack)
           || interface.flags().testFlag(QNetworkInterface::IsPointToPoint)
              )
        {
            qDebug() << Q_FUNC_INFO << "Skipping interface" << interface.humanReadableName();
            continue;
        }

        foreach(QNetworkAddressEntry addressEntry, interface.addressEntries())
        {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
            if(stopping)
                return;
            if(addressEntry.ip().protocol()!=QAbstractSocket::IPv4Protocol)
            {
                qDebug() << Q_FUNC_INFO << "Skipping address" << addressEntry.ip().toString();
                continue;
            }

            quint32 myaddress = addressEntry.ip().toIPv4Address();
            quint32 netmask = addressEntry.netmask().toIPv4Address();
            quint32 network = myaddress & netmask;
            quint32 address = network + 1;
            quint32 broadcast = addressEntry.broadcast().toIPv4Address();

            while((address & netmask) == network)
            {
                if(stopping)
                    return;
    
                if(address == broadcast)
                    break;

                QHostAddress hostaddress(address);

#ifdef Q_OS_MAC
                while(pingProcesses.count()>30)
#else
                while(pingProcesses.count()>50)
#endif
                {
                    foreach(QProcess *process, pingProcesses)
                    {
                        if(process->state()==QProcess::NotRunning)
                        {
                            qDebug() << Q_FUNC_INFO << "Ping finished" << QString::fromLocal8Bit(process->readAll());
                            pingProcesses.removeOne(process);
                            process->close();
                            process->deleteLater();
                        }
                    }

                    QCoreApplication::processEvents(QEventLoop::AllEvents, 200);
                    if(stopping)
                        return;
                }

                qDebug() << Q_FUNC_INFO << "Sending ping from" << addressEntry.ip().toString() << "to" << hostaddress.toString();

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
                    emit(error(Q_FUNC_INFO, tr("Could not start ping :\n%1").arg(process->errorString())));
                    break;
                }

                address++;
            }
        }
    }

    while(pingProcesses.count()>0)
    {
        foreach(QProcess *process, pingProcesses)
        {

            if(process->state()==QProcess::NotRunning)
            {
                qDebug() << Q_FUNC_INFO << "Ping finished" << QString::fromLocal8Bit(process->readAll());
                pingProcesses.removeOne(process);
                process->close();
                process->deleteLater();
            }
        }

        QCoreApplication::processEvents(QEventLoop::AllEvents, 200);
        if(stopping)
            return;
    }

    qDebug() << Q_FUNC_INFO << "Fin emission des requetes ping";

    allRequestsSent=true;
}

void PingSearchWorker::stop()
{
    qDebug() << Q_FUNC_INFO;

    checkResultsTimer->stop();

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    if(arpTableProcess->state()!=QProcess::NotRunning)
        arpTableProcess->kill();
#endif

    foreach(int id, lookupIDs.keys())
        QHostInfo::abortHostLookup(id);

    SearchWorker::stop();
}

#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
void PingSearchWorker::checkResults()
{
    qDebug() << Q_FUNC_INFO;

    if(stopping)
        return;

    QMutexLocker locker(&arpMutex);

    if(arpTableProcess && arpTableProcess->state()!=QProcess::NotRunning)
    {
        checkResultsTimer->start();
        return;
    }

    if(arpTableProcess)
    {
        arpTableProcess->close();
        arpTableProcess->deleteLater();
    }
    arpTableProcess = new QProcess(this);
    connect(arpTableProcess, SIGNAL(finished(int)), this, SLOT(gotArpResults(int)));

    arpTableProcess->start("arp", QStringList("-a"));

    if(!arpTableProcess->waitForStarted())
        emit(error(Q_FUNC_INFO, tr("Could not run arp tool.\n%1").arg(arpTableProcess->errorString())));
}

void PingSearchWorker::gotArpResults(int)
{
    qDebug() << Q_FUNC_INFO;

    if(stopping)
        return;

    QMutexLocker locker(&arpMutex);

#ifdef Q_OS_WIN
    QRegExp rx("^ *([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}) *([^ -]+-[^ -]+-[^ -]+-[^ -]+-[^ -]+-[^ -]+).*$");
#else
    QRegExp rx("^? *\\(([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})\\) at ([^ :]+:[^ :]+:[^ :]+:[^ :]+:[^ :]+).*$");
#endif

    while(!arpTableProcess->atEnd())
    {
        QString line(arpTableProcess->readLine());
        int pos = rx.indexIn(line);
        if(pos<0)
            continue;

        QStringList list = rx.capturedTexts();
        if(list.count()!=3)
            continue;

#ifdef Q_OS_WIN
        QString mac = list.at(2).split("-").join(":").toUpper();
#else
        QString mac;
        foreach(QString part, list.at(2).split(":"))
        {
            mac.append(part.rightJustified(2, '0').toUpper());
            mac.append(':');
        }
        mac.chop(1);
#endif

        if(!mac.compare("00:00:00:00:00:00") || !mac.compare("FF:FF:FF:FF:FF:FF") || mac.startsWith("01:00:5E"))
            continue;

        if(checkedMACs.contains(mac))
            continue;

        checkedMACs << mac;

        allRequestsSent=false;
        lookupIDs.insert(QHostInfo::lookupHost(list.at(1), this, SLOT(lookedUp(QHostInfo))), mac);
    }

    if(webPagesToCheck<=0 && lookupIDs.isEmpty())
        emit(finished());
    else if(!stopping)
        checkResultsTimer->start();
}
#endif

#ifdef Q_OS_LINUX
void PingSearchWorker::checkResults()
{
    qDebug() << Q_FUNC_INFO;

    if(stopping)
        return;

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

            if(!mac.compare("00:00:00:00:00:00") || !mac.compare("FF:FF:FF:FF:FF:FF") || mac.startsWith("01:00:5E"))
                continue;

            if(checkedMACs.contains(mac))
                continue;

            checkedMACs << mac;

            allRequestsSent=false;
            lookupIDs.insert(QHostInfo::lookupHost(list.at(1), this, SLOT(lookedUp(QHostInfo))), mac);
        }
    }
    else
        emit(error(Q_FUNC_INFO, tr("Could not open /proc/net/arp: %1").arg(arpTable.errorString())));

    if(webPagesToCheck<=0 && lookupIDs.isEmpty())
       emit(finished());
    else if(!stopping)
        checkResultsTimer->start();
}
#endif

void PingSearchWorker::lookedUp(QHostInfo hostInfo)
{
    Host thisHost;
    thisHost.name = hostInfo.hostName().toLower();
    thisHost.ip = hostInfo.addresses().at(0).toString();
    thisHost.desc = tr("UDP + ARP cache (MAC : %1)").arg(lookupIDs.value(hostInfo.lookupId()));
    lookupIDs.remove(hostInfo.lookupId());

    qDebug() << Q_FUNC_INFO << "Trying to find jeedom website on" << thisHost.name << "(" << thisHost.ip << ")";

    if(stopping)
        return;

#ifdef HAS_SSL_SUPPORT
    checkWebPage(&thisHost,QString("https://%1/").arg(thisHost.name));
    checkWebPage(&thisHost,QString("https://%1/jeedom/").arg(thisHost.name));
#endif
    checkWebPage(&thisHost,QString("http://%1/").arg(thisHost.name));
    checkWebPage(&thisHost,QString("http://%1/jeedom/").arg(thisHost.name));

    if(lookupIDs.isEmpty())
    {
        allRequestsSent=true;
        if(webPagesToCheck<=0)
            emit(finished());
    }
}
