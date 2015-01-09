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

    QHostInfo hostInfo = QHostInfo::fromName("jeedom");

    bool multipleIPs = hostInfo.addresses().count()>1;
    foreach(QHostAddress address, hostInfo.addresses())
    {
        Host thisHost;
        thisHost.name = hostInfo.hostName();
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

    allRequestsSent=true;

    if(webPagesToCheck<=0)
    {
        emit(finished());
    }

    qDebug() << Q_FUNC_INFO;
}
