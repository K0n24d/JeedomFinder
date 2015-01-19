#ifndef DNSLOOKUPSEARCHWORKER_H
#define DNSLOOKUPSEARCHWORKER_H

#include "searchworker.h"
#include <QHostInfo>

class DNSLookupSearchWorker : public SearchWorker
{
    Q_OBJECT

public:
    DNSLookupSearchWorker(QObject *parent = NULL);
    ~DNSLookupSearchWorker();

protected:
    QHash<int, QHostInfo> lookupIDs;

protected slots:
    void lookedUp(QHostInfo hostInfo);
    void reverseLookedUp(QHostInfo hostInfo);

public slots:
    void discover();
    void stop();
};

#endif // DNSLOOKUPSEARCHWORKER_H
