#ifndef BONJOURSEARCHWORKER_H
#define BONJOURSEARCHWORKER_H

#include <QObject>
#include "bonjourrecord.h"

class BonjourServiceBrowser;
class BonjourServiceResolver;

class BonjourSearchWorker : public QObject
{
    Q_OBJECT

public:
    BonjourSearchWorker(QObject *parent = 0);

private slots:
    void updateRecords(const QList<BonjourRecord> &list);

protected:
    BonjourServiceBrowser *bonjourBrowser;
    BonjourServiceResolver *bonjourResolver;

signals:
    void finished();
    void error(const QString, const QString);
public slots:
    void discover();
    void stop();
};

#endif // BONJOURSEARCHWORKER_H
