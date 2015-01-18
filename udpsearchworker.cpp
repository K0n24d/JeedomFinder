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
    SearchWorker(parent), stopping(false)
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
}

void UdpSearchWorker::discover()
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
                if(address == broadcast)
                    break;

                QHostAddress hostaddress(address);

                qDebug() << Q_FUNC_INFO << "Sending udp packet from" << addressEntry.ip().toString() << "to" << hostaddress.toString();

                socket.writeDatagram(QByteArray("D"), hostaddress, 9999);

                address++;
            }
        }
    }

    qDebug() << Q_FUNC_INFO << "Fin emission des paquets udp";

    allRequestsSent=true;
}

void UdpSearchWorker::stop()
{
    qDebug() << Q_FUNC_INFO;

    checkResultsTimer->stop();

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    if(arpTableProcess->state()!=QProcess::NotRunning)
        arpTableProcess->kill();
#endif

    stopping = true;
}

#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
void UdpSearchWorker::checkResults()
{
    qDebug() << Q_FUNC_INFO;

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

//        if(mac.startsWith("B8:27:EB"))
        {
            qDebug() << Q_FUNC_INFO << "Trying to find jeedom website on" << list.at(1);
            checkedMACs << mac;
            QHostInfo hostInfo = QHostInfo::fromName(list.at(1));

            Host thisHost;
            thisHost.name = hostInfo.hostName();
            thisHost.ip = list.at(1);
            thisHost.desc = tr("UDP + ARP cache (MAC : %1)").arg(mac);

            checkWebPage(&thisHost,QString("https://%1/").arg(thisHost.name));
            checkWebPage(&thisHost,QString("https://%1/jeedom/").arg(thisHost.name));
            checkWebPage(&thisHost,QString("http://%1/").arg(thisHost.name));
            checkWebPage(&thisHost,QString("http://%1/jeedom/").arg(thisHost.name));
        }
    }

    if(webPagesToCheck<=0 && allRequestsSent)
    {
        emit(finished());
    }
    else if(!stopping)
        checkResultsTimer->start();
}
#endif

#ifdef Q_OS_LINUX
void UdpSearchWorker::checkResults()
{
    qDebug() << Q_FUNC_INFO;

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

//            if(mac.startsWith("B8:27:EB"))
            {
                qDebug() << Q_FUNC_INFO << "Trying to find jeedom website on" << list.at(1);
                checkedMACs << mac;
                QHostInfo hostInfo = QHostInfo::fromName(list.at(1));

                Host thisHost;
                thisHost.name = hostInfo.hostName().toLower();
                thisHost.ip = list.at(1);
                thisHost.desc = tr("UDP + ARP cache (MAC : %1)").arg(mac);

                checkWebPage(&thisHost,QString("https://%1/").arg(thisHost.name));
                checkWebPage(&thisHost,QString("https://%1/jeedom/").arg(thisHost.name));
                checkWebPage(&thisHost,QString("http://%1/").arg(thisHost.name));
                checkWebPage(&thisHost,QString("http://%1/jeedom/").arg(thisHost.name));
            }
        }
    }
    else
        emit(error(Q_FUNC_INFO, tr("Impossible d'ouvrir /proc/net/arp: %1").arg(arpTable.errorString())));

    if(webPagesToCheck<=0 && allRequestsSent)
    {
        emit(finished());
    }
    else if(!stopping)
        checkResultsTimer->start();
}
#endif
