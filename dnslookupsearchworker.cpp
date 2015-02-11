#include "dnslookupsearchworker.h"
#include <QHostInfo>
#include <QtDebug>

DNSLookupSearchWorker::DNSLookupSearchWorker(QObject *parent) :
    SearchWorker(parent)
{
    qDebug() << Q_FUNC_INFO;
}

DNSLookupSearchWorker::~DNSLookupSearchWorker()
{
}

void DNSLookupSearchWorker::discover()
{
    qDebug() << Q_FUNC_INFO << "Start";

    allRequestsSent = false;

    lookupIDs.insert(QHostInfo::lookupHost("jeedom.local", this, SLOT(lookedUp(QHostInfo))), QHostInfo());
    lookupIDs.insert(QHostInfo::lookupHost("jeedom", this, SLOT(lookedUp(QHostInfo))), QHostInfo());
}

void DNSLookupSearchWorker::lookedUp(QHostInfo hostInfo)
{
    qDebug() << Q_FUNC_INFO << hostInfo.hostName().toLower();

    lookupIDs.remove(hostInfo.lookupId());

    foreach(QHostAddress address, hostInfo.addresses())
    {
        qDebug() << Q_FUNC_INFO << "Tenter un reverse lookup pour" << address.toString();
        lookupIDs.insert(QHostInfo::lookupHost(address.toString(), this, SLOT(reverseLookedUp(QHostInfo))), hostInfo);
    }

    if(lookupIDs.isEmpty())
    {
        allRequestsSent=true;
        if(webPagesToCheck<=0)
            emit(finished());
    }
}

void DNSLookupSearchWorker::reverseLookedUp(QHostInfo reverseLookup)
{
    QHostInfo hostInfo = lookupIDs.value(reverseLookup.lookupId());

    qDebug() << Q_FUNC_INFO << hostInfo.hostName().toLower() << reverseLookup.hostName();

    bool multipleIPs = hostInfo.addresses().count()>1;
    Host thisHost;

    if(reverseLookup.hostName() == reverseLookup.addresses().at(0).toString())
    {
        thisHost.name = hostInfo.hostName().toLower();
        qDebug() << Q_FUNC_INFO << "Utilisation du nom d'hôte d'origine" << thisHost.name;
    }
    else
    {
        thisHost.name = reverseLookup.hostName().toLower();
        qDebug() << Q_FUNC_INFO << "Utilisation du reverse" << thisHost.name;
    }
    thisHost.ip = reverseLookup.addresses().at(0).toString();
    thisHost.desc = tr("Recherche DNS (%1)").arg(hostInfo.hostName());

    QString urlHostName(thisHost.name);
    if(multipleIPs)
    {
        urlHostName = thisHost.ip;
        qDebug() << "Adresses ip multiples";
    }

#ifdef HAS_SSL_SUPPORT
    checkWebPage(&thisHost,QString("https://%1/").arg(urlHostName));
    checkWebPage(&thisHost,QString("https://%1/jeedom/").arg(urlHostName));
#endif
    checkWebPage(&thisHost,QString("http://%1/").arg(urlHostName));
    checkWebPage(&thisHost,QString("http://%1/jeedom/").arg(urlHostName));

    lookupIDs.remove(reverseLookup.lookupId());
    if(lookupIDs.isEmpty())
    {
        allRequestsSent=true;
        if(webPagesToCheck<=0)
            emit(finished());
    }
}

void DNSLookupSearchWorker::stop()
{
    qDebug() << Q_FUNC_INFO;

    foreach(int id, lookupIDs.keys())
        QHostInfo::abortHostLookup(id);

    SearchWorker::stop();
}
