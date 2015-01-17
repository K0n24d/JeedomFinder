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

    lookup("jeedom.local");
    lookup("jeedom");

    allRequestsSent=true;

    if(webPagesToCheck<=0)
    {
        emit(finished());
    }

    qDebug() << Q_FUNC_INFO;
}

void DNSLookupSearchWorker::lookup(const QString &hostname)
{
    QHostInfo hostInfo = QHostInfo::fromName(hostname);

    bool multipleIPs = hostInfo.addresses().count()>1;
    foreach(QHostAddress address, hostInfo.addresses())
    {
        Host thisHost;
        QHostInfo reverseLookup = QHostInfo::fromName(address.toString());
        if(reverseLookup.hostName() == address.toString())
            thisHost.name = hostInfo.hostName().toLower();
        else
            thisHost.name = reverseLookup.hostName().toLower();
        thisHost.ip = address.toString();
        thisHost.desc = tr("Recherche DNS");

        QString urlHostName(thisHost.name);
        if(multipleIPs)
            urlHostName = thisHost.ip;

        checkWebPage(&thisHost,QString("https://%1/").arg(urlHostName));
        checkWebPage(&thisHost,QString("https://%1/jeedom/").arg(urlHostName));
        checkWebPage(&thisHost,QString("http://%1/").arg(urlHostName));
        checkWebPage(&thisHost,QString("http://%1/jeedom/").arg(urlHostName));
    }
}
