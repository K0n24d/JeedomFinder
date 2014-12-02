#include "bonjoursearchworker.h"
#include "bonjourservicebrowser.h"
#include "bonjourserviceresolver.h"

#include <QRegExp>

BonjourSearchWorker::BonjourSearchWorker(const QString &searchServiceType, QObject *parent)
    : SearchWorker(parent), serviceType(searchServiceType)
{
    bonjourBrowser = new BonjourServiceBrowser(this);

    connect(bonjourBrowser, SIGNAL(currentBonjourRecordsChanged(const QList<BonjourRecord> &)),
            this, SLOT(updateRecords(const QList<BonjourRecord> &)));
}

void BonjourSearchWorker::discover()
{
    bonjourBrowser->browseForServiceType(serviceType);
}

void BonjourSearchWorker::stop()
{
}

void BonjourSearchWorker::updateRecords(const QList<BonjourRecord> &list)
{
    foreach (BonjourRecord record, list)
    {
        BonjourServiceResolver *bonjourResolver = new BonjourServiceResolver(this);

        bonjourResolvers.insert(bonjourResolver, record);
        connect(bonjourResolver, SIGNAL(bonjourRecordResolved(QHostInfo,int,QString)),
                this, SLOT(recordResolved(QHostInfo,int,QString)));

        bonjourResolver->resolveBonjourRecord(record);
    }
}

void BonjourSearchWorker::recordResolved(const QHostInfo &hostInfo, int port, const QString &txt)
{
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
        thisHost->name = hostInfo.hostName();
        while(thisHost->name.endsWith('.'))
            thisHost->name.chop(1);
        thisHost->ip = hostInfo.addresses().isEmpty()?QString():hostInfo.addresses().at(0).toString();
        thisHost->desc = tr("Name: %1, Type: %2, Domain: %3, Port: %4").arg(record.serviceName).arg(record.registeredType).arg(record.replyDomain).arg(port);
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

    if(bonjourResolvers.isEmpty())
        emit(finished());
}
