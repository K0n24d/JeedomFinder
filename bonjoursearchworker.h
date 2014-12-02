#ifndef BONJOURSEARCHWORKER_H
#define BONJOURSEARCHWORKER_H

#include "searchworker.h"
#include "bonjourrecord.h"
#include <QHostInfo>
#include <QMap>

class BonjourServiceBrowser;
class BonjourServiceResolver;

class BonjourSearchWorker : public SearchWorker
{
    Q_OBJECT

public:
    BonjourSearchWorker(const QString &searchServiceType, QObject *parent = 0);

private slots:
    void updateRecords(const QList<BonjourRecord> &list);
    void recordResolved(const QHostInfo &hostInfo, int port, const QString &txt);

protected:
    QString serviceType;
    BonjourServiceBrowser *bonjourBrowser;
    QMap<BonjourServiceResolver*, BonjourRecord> bonjourResolvers;

public slots:
    void discover();
    void stop();
};

#endif // BONJOURSEARCHWORKER_H
