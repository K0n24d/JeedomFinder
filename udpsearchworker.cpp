#include "udpsearchworker.h"
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

UdpSearchWorker::UdpSearchWorker(QObject *parent) :
    SearchWorker(parent)
{
    qDebug() << Q_FUNC_INFO;

    checkResultsTimer=NULL;
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    arpTableProcess=NULL;
#endif
    manager=NULL;
}

UdpSearchWorker::~UdpSearchWorker()
{
    if(checkResultsTimer)
    {
        checkResultsTimer->stop();
        checkResultsTimer->deleteLater();
    }

    foreach(QUdpSocket * socket, sockets)
    {
        socket->close();
        socket->deleteLater();
        sockets.removeOne(socket);
    }
}

void UdpSearchWorker::discover()
{
    qDebug() << Q_FUNC_INFO;

    allRequestsSent=false;

    checkedMACs.clear();

    if(checkResultsTimer)
    {
        checkResultsTimer->stop();
        checkResultsTimer->deleteLater();
    }

    while(sockets.count()<10)
    {
        QUdpSocket *socket = new QUdpSocket(this);
        socket->bind();
        sockets << socket;
        qDebug() << Q_FUNC_INFO << "Nombre de sockets ouverts" << sockets.count();
    }
    QUdpSocket *socket = sockets.first();


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
                QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                if(stopping)
                    return;
    
                if(address == broadcast)
                    break;

                if(address != myaddress)
                {
                    QHostAddress hostaddress(address);

                    qDebug() << Q_FUNC_INFO << "Sending udp packet from" << addressEntry.ip().toString() << "to" << hostaddress.toString();


                    /*
                    while(socket->writeDatagram(QByteArray("D"), hostaddress, 9999)!=1)
                    {
                        if(stopping)
                            return;
                        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                    }
                    */
                    while(socket->writeDatagram(QByteArray("D"), hostaddress, 9999)!=1)
                    {
                        int index = sockets.indexOf(socket)+1;
                        if(index==sockets.count())
                            index=0;
                        socket=sockets[index];

                        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                        if(stopping)
                            return;
                    }
                }

                address++;
            }
        }
    }

    qDebug() << Q_FUNC_INFO << "Fin emission des paquets udp";

    checkResultsTimer = new QTimer(this);
    checkResultsTimer->setInterval(1000);
    checkResultsTimer->setSingleShot(true);
    connect(checkResultsTimer, SIGNAL(timeout()), this, SLOT(checkResults()));
    checkResultsTimer->start();
}

void UdpSearchWorker::stop()
{
    qDebug() << Q_FUNC_INFO;

    //checkResultsTimer->stop();

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    if(arpTableProcess->state()!=QProcess::NotRunning)
        arpTableProcess->kill();
#endif

    /*
    foreach(QUdpSocket * socket, sockets)
    {
        socket->close();
        socket->deleteLater();
        sockets.removeOne(socket);
    }
    */

    foreach(int id, lookupIDs.keys())
        QHostInfo::abortHostLookup(id);

    SearchWorker::stop();
}

#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
void UdpSearchWorker::checkResults()
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
        emit(error(Q_FUNC_INFO, tr("Impossible de lancer l'utilitaire arp.\n%1").arg(arpTableProcess->errorString())));
}

void UdpSearchWorker::gotArpResults(int)
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
void UdpSearchWorker::checkResults()
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
        emit(error(Q_FUNC_INFO, tr("Impossible d'ouvrir /proc/net/arp: %1").arg(arpTable.errorString())));

    if(webPagesToCheck<=0 && lookupIDs.isEmpty())
       emit(finished());
    else if(!stopping)
        checkResultsTimer->start();
}
#endif

void UdpSearchWorker::lookedUp(QHostInfo hostInfo)
{
    Host thisHost;
    thisHost.name = hostInfo.hostName().toLower();
    thisHost.ip = hostInfo.addresses().at(0).toString();
    thisHost.desc = tr("UDP + ARP cache (MAC : %1)").arg(lookupIDs.value(hostInfo.lookupId()));
    lookupIDs.remove(hostInfo.lookupId());

    qDebug() << Q_FUNC_INFO << "Trying to find jeedom website on" << thisHost.name << "(" << thisHost.ip << ")";

    if(stopping)
        return;

    checkWebPage(&thisHost,QString("https://%1/").arg(thisHost.name));
    checkWebPage(&thisHost,QString("https://%1/jeedom/").arg(thisHost.name));
    checkWebPage(&thisHost,QString("http://%1/").arg(thisHost.name));
    checkWebPage(&thisHost,QString("http://%1/jeedom/").arg(thisHost.name));

    if(lookupIDs.isEmpty())
    {
        allRequestsSent=true;
        if(webPagesToCheck<=0)
            emit(finished());
    }
}
