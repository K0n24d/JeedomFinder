#ifndef DNSLOOKUPSEARCHWORKER_H
#define DNSLOOKUPSEARCHWORKER_H

#include "searchworker.h"

class DNSLookupSearchWorker : public SearchWorker
{
    Q_OBJECT

public:
    DNSLookupSearchWorker(QObject *parent = NULL);
    ~DNSLookupSearchWorker();

protected:
    void lookup(const QString &hostname);

public slots:
    void discover();
};

#endif // DNSLOOKUPSEARCHWORKER_H
