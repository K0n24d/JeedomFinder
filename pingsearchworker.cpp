#include "pingsearchworker.h"
#include <QProcess>
#include <QNetworkInterface>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QHostInfo>
#include <QNetworkReply>

PingSearchWorker::PingSearchWorker(QObject *parent) :
    SearchWorker(parent), stopping(false)
{
    checkResultsTimer=NULL;
    manager=NULL;
}

PingSearchWorker::~PingSearchWorker()
{
    if(checkResultsTimer)
    {
        checkResultsTimer->stop();
        checkResultsTimer->deleteLater();
    }

    if(manager)
        manager->deleteLater();

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

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    arpTableProcess = new QProcess(this);
    connect(arpTableProcess, SIGNAL(finished(int)), this, SLOT(gotArpResults(int)));
#endif

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

    if(manager)
        manager->deleteLater();
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

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

#ifdef Q_OS_MAC
                while(pingProcesses.count()>40)
#else
                while(pingProcesses.count()>50)
#endif
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
                    emit(error(Q_FUNC_INFO, tr("Impossible de lancer ping :\n%1").arg(process->errorString())));
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
    checkResultsTimer->stop();

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    if(arpTableProcess->state()!=QProcess::NotRunning)
        arpTableProcess->kill();
#endif

    stopping = true;
}

#ifdef Q_OS_WIN
void PingSearchWorker::checkResults()
{
    if(arpTableProcess->state()!=QProcess::NotRunning)
    {
        checkResultsTimer->start();
        return;
    }

    arpTableProcess->start("arp", QStringList("-a"));

    if(!arpTableProcess->waitForStarted())
        emit(error(Q_FUNC_INFO, tr("Impossible de lancer l'utilitaire arp.\n%1").arg(arpTableProcess->errorString())));
}

void PingSearchWorker::gotArpResults(int)
{
    QRegExp rx("^ *([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}) *([^ -]+-[^ -]+-[^ -]+-[^ -]+-[^ -]+-[^ -]+).*$");

    while(!arpTableProcess->atEnd())
    {
        QString line(arpTableProcess->readLine());
        int pos = rx.indexIn(line);
        if(pos<0)
            continue;

        QStringList list = rx.capturedTexts();
        if(list.count()!=3)
            continue;

        QString mac = list.at(2).split("-").join(":").toUpper();

        if(checkedMACs.contains(mac))
            continue;

        if(mac.startsWith("B8:27:EB"))
        {
            checkedMACs << mac;
            QHostInfo hostInfo = QHostInfo::fromName(list.at(1));

            Host thisHost;
            thisHost.name = hostInfo.hostName();
            thisHost.ip = list.at(1);
            thisHost.desc = tr("MAC : %1").arg(mac);

            checkWebPage(&thisHost,QString("https://%1/").arg(thisHost.name));
            checkWebPage(&thisHost,QString("https://%1/jeedom/").arg(thisHost.name));
            checkWebPage(&thisHost,QString("http://%1/").arg(thisHost.name));
            checkWebPage(&thisHost,QString("http://%1/jeedom/").arg(thisHost.name));
        }
    }

    checkResultsTimer->start();
}
#endif

#ifdef Q_OS_MAC
void PingSearchWorker::checkResults()
{
    if(arpTableProcess->state()!=QProcess::NotRunning)
    {
        checkResultsTimer->start();
        return;
    }

    arpTableProcess->start("arp", QStringList("-a"));

    if(!arpTableProcess->waitForStarted())
        emit(error(Q_FUNC_INFO, tr("Impossible de lancer l'utilitaire arp.\n%1").arg(arpTableProcess->errorString())));
}

void PingSearchWorker::gotArpResults(int)
{
    QRegExp rx("^? *\\(([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})\\) at ([^ :]+:[^ :]+:[^ :]+:[^ :]+:[^ :]+).*$");

    while(!arpTableProcess->atEnd())
    {
        QString line(arpTableProcess->readLine());
        int pos = rx.indexIn(line);
        if(pos<0)
            continue;

        QStringList list = rx.capturedTexts();
        if(list.count()!=3)
            continue;

        QString mac;
        foreach(QString part, list.at(2).split(":"))
        {
            mac.append(part.rightJustified(2, '0').toUpper());
            mac.append(':');
        }
        mac.chop(1);

        if(checkedMACs.contains(mac))
            continue;

        if(mac.startsWith("B8:27:EB"))
        {
            checkedMACs << mac;
            QHostInfo hostInfo = QHostInfo::fromName(list.at(1));

            Host thisHost;
            thisHost.name = hostInfo.hostName();
            thisHost.ip = list.at(1);
            thisHost.desc = tr("MAC : %1").arg(mac);

            checkWebPage(&thisHost,QString("https://%1/").arg(thisHost.name));
            checkWebPage(&thisHost,QString("https://%1/jeedom/").arg(thisHost.name));
            checkWebPage(&thisHost,QString("http://%1/").arg(thisHost.name));
            checkWebPage(&thisHost,QString("http://%1/jeedom/").arg(thisHost.name));
        }
    }

    checkResultsTimer->start();
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

            if(checkedMACs.contains(mac))
                continue;

            if(mac.startsWith("B8:27:EB"))
            {
                checkedMACs << mac;
                QHostInfo hostInfo = QHostInfo::fromName(list.at(1));

                Host thisHost;
                thisHost.name = hostInfo.hostName();
                thisHost.ip = list.at(1);
                thisHost.desc = tr("MAC : %1").arg(mac);

                checkWebPage(&thisHost,QString("https://%1/").arg(thisHost.name));
                checkWebPage(&thisHost,QString("https://%1/jeedom/").arg(thisHost.name));
                checkWebPage(&thisHost,QString("http://%1/").arg(thisHost.name));
                checkWebPage(&thisHost,QString("http://%1/jeedom/").arg(thisHost.name));
            }
        }
    }
    else
        emit(error(Q_FUNC_INFO, tr("Impossible d'ouvrir /proc/net/arp: %1").arg(arpTable.errorString())));

    if(!stopping)
        checkResultsTimer->start();
}
#endif

void PingSearchWorker::checkWebPage(const Host *host, QString url)
{
    Host * thisHost = new Host(host, this);
    if(thisHost->url.isEmpty())
        thisHost->url = url;
    QNetworkRequest request(url);
    request.setOriginatingObject(thisHost);
    manager->get(request);
}

void PingSearchWorker::replyFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    Host *thisHost = static_cast<Host*>(reply->request().originatingObject());
    if(!thisHost)
        return;

    if (reply->error() == QNetworkReply::NoError)
    {
        QVariant redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        if(redirect.isValid())
        {
            QUrl newUrl(redirect.toUrl());
            if (newUrl.isRelative())
                newUrl = reply->request().url().resolved(newUrl);
            thisHost->desc.append(tr(" Redirect : %1").arg(newUrl.toString()));
            checkWebPage(thisHost, newUrl.toString());
            thisHost->deleteLater();
            return;
        }
        QString page(reply->readAll());
        if(page.contains("<title>Jeedom</title>", Qt::CaseInsensitive))
            emit(host(thisHost));
    }
}
