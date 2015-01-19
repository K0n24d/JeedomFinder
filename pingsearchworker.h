#ifndef PINGSEARCHWORKER_H
#define PINGSEARCHWORKER_H

#include "searchworker.h"
#include <QTimer>
#include <QHostInfo>

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
#include <QProcess>
#include <QMutex>
#else
class QProcess;
#endif

class PingSearchWorker : public SearchWorker
{
    Q_OBJECT
public:
    explicit PingSearchWorker(QObject *parent = 0);
    ~PingSearchWorker();

protected:
    QList<QProcess*> pingProcesses;
    QTimer *checkResultsTimer;
    QList<QString> checkedMACs;
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    QProcess *arpTableProcess;
    QMutex arpMutex;
#endif
    QHash<int, QString> lookupIDs;

protected slots:
    void checkResults();
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    void gotArpResults(int);
#endif
    void lookedUp(QHostInfo hostInfo);

public slots:
    void discover();
    void stop();
};

#endif // PINGSEARCHWORKER_H
