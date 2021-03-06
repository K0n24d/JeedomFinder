#include "bonjoursearchworker.h"
#include "bonjourserviceresolver.h"

#include <QRegExp>
#include <QtDebug>
#include <QTimerEvent>

BonjourSearchWorker::BonjourSearchWorker(const QString &searchServiceType, QObject *parent)
    : SearchWorker(parent), serviceType(searchServiceType)
{
    qDebug() << Q_FUNC_INFO << "Start";

    bonjourBrowser = new BonjourServiceBrowser(this);

    connect(bonjourBrowser, SIGNAL(currentBonjourRecordsChanged(const QList<BonjourRecord> &)),
            this, SLOT(updateRecords(const QList<BonjourRecord> &)));

    connect(bonjourBrowser, SIGNAL(error(DNSServiceErrorType)),
            this, SLOT(bonjourError(DNSServiceErrorType)));


    qDebug() << Q_FUNC_INFO << "End";
}

void BonjourSearchWorker::discover()
{
    qDebug() << Q_FUNC_INFO;

    bonjourBrowser->browseForServiceType(serviceType);
    bonjourBrowseTimeout = startTimer(20000);
}

void BonjourSearchWorker::bonjourError(DNSServiceErrorType err)
{
    qWarning() << Q_FUNC_INFO << err;
    stop();
}

void BonjourSearchWorker::timerEvent(QTimerEvent *event)
{
    qWarning() << Q_FUNC_INFO  << event->timerId();

    if(event->timerId()==bonjourBrowseTimeout)
    {
        killTimer(bonjourBrowseTimeout);
        stop();
    }
}

void BonjourSearchWorker::updateRecords(const QList<BonjourRecord> &list)
{
    qDebug() << Q_FUNC_INFO;

    if(list.isEmpty())
    {
        qWarning() << Q_FUNC_INFO << "Aucun enregistrement MDNS";
        emit(finished());
    }

    foreach (BonjourRecord record, list)
    {
        qDebug() << Q_FUNC_INFO << record.registeredType << record.serviceName  << record.replyDomain;

        BonjourServiceResolver *bonjourResolver = new BonjourServiceResolver(this);

        bonjourResolvers.insert(bonjourResolver, record);
        connect(bonjourResolver, SIGNAL(bonjourRecordResolved(QHostInfo,int,QString)),
                this, SLOT(recordResolved(QHostInfo,int,QString)));

        bonjourResolver->resolveBonjourRecord(record);
    }
}

void BonjourSearchWorker::recordResolved(const QHostInfo &hostInfo, int port, const QString &txt)
{
    qDebug() << Q_FUNC_INFO << hostInfo.hostName() << hostInfo.addresses() << port << txt;

    BonjourServiceResolver *resolver = static_cast<BonjourServiceResolver *>(sender());
    BonjourRecord record = bonjourResolvers.value(resolver);
    bonjourResolvers.remove(resolver);

    if(record.serviceName.contains("jeedom", Qt::CaseInsensitive))
    {
        QString path;
        QRegExp rxPath("path\\s*=\\s*(.+)", Qt::CaseInsensitive);
        if(rxPath.indexIn(txt, 0)>=0)
            path=rxPath.cap(1);
        if(!path.startsWith('/'))
            path.prepend('/');

        Host *thisHost = new Host(this);
        thisHost->name = hostInfo.hostName().toLower();
        while(thisHost->name.endsWith('.'))
            thisHost->name.chop(1);
        thisHost->ip = hostInfo.addresses().isEmpty()?QString():hostInfo.addresses().at(0).toString();
        thisHost->desc = tr("Zeroconf (Name: %1, Type: %2, Domain: %3, Port: %4)").arg(record.serviceName).arg(record.registeredType).arg(record.replyDomain).arg(port);
        if(serviceType.startsWith("_https"))
        {
            if(port==443)
                thisHost->url = QString("https://%1%2").arg(thisHost->name).arg(path);
            else
                thisHost->url = QString("https://%1:%2%3").arg(thisHost->name).arg(port).arg(path);
        }
        else
        {
            if(port==80)
                thisHost->url = QString("http://%1%2").arg(thisHost->name).arg(path);
            else
                thisHost->url = QString("http://%1:%2%3").arg(thisHost->name).arg(port).arg(path);
        }
        emit(host(thisHost));
    }

    resolver->deleteLater();

    hasFinished();
}

bool BonjourSearchWorker::available()
{
    QLibrary libdns_sd(DNS_SD_LIB_NAME);
    return(libdns_sd.load());
}

bool BonjourSearchWorker::hasFinished()
{
    if(stopping)
        return true;

    if(bonjourResolvers.isEmpty())
    {
        stop();
        return true;
    }

    return false;
}
