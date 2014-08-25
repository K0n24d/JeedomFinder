#include "bonjoursearchworker.h"
#include "bonjourservicebrowser.h"
#include "bonjourserviceresolver.h"

BonjourSearchWorker::BonjourSearchWorker(QObject *parent)
    : SearchWorker(parent)
{
    bonjourBrowser = new BonjourServiceBrowser(this);

    connect(bonjourBrowser, SIGNAL(currentBonjourRecordsChanged(const QList<BonjourRecord> &)),
            this, SLOT(updateRecords(const QList<BonjourRecord> &)));
}

void BonjourSearchWorker::discover()
{
    bonjourBrowser->browseForServiceType(QString("_printer._tcp"));
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
        connect(bonjourResolver, SIGNAL(bonjourRecordResolved(QHostInfo,int)),
                this, SLOT(recordResolved(QHostInfo,int)));

        bonjourResolver->resolveBonjourRecord(record);
    }
}

void BonjourSearchWorker::recordResolved(const QHostInfo &hostInfo, int port)
{
    BonjourServiceResolver *resolver = static_cast<BonjourServiceResolver *>(sender());
    BonjourRecord record = bonjourResolvers.value(resolver);
    resolver->deleteLater();
    bonjourResolvers.remove(resolver);

    Host thisHost;
    thisHost.name = hostInfo.hostName();
    thisHost.ip = hostInfo.addresses().isEmpty()?QString():hostInfo.addresses().at(0).toString();
    thisHost.desc = tr("Name: %1, Type: %2, Domain: %3, Port: %4").arg(record.serviceName).arg(record.registeredType).arg(record.replyDomain).arg(port);
    thisHost.url = QString("http://%1:%2/jeedom").arg(thisHost.ip).arg(port);
    emit(host(thisHost));

    if(bonjourResolvers.isEmpty())
        emit(finished());
}
