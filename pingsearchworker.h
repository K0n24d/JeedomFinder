#ifndef PINGSEARCHWORKER_H
#define PINGSEARCHWORKER_H

#include "searchworker.h"
#include <QTimer>

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
    bool stopping;
    QList<QProcess*> pingProcesses;
    QTimer *checkResultsTimer;
    QList<QString> checkedMACs;
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    QProcess *arpTableProcess;
    QMutex arpMutex;
#endif

protected slots:
    void checkResults();
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    void gotArpResults(int);
#endif

public slots:
    void discover();
    void stop();
};

#endif // PINGSEARCHWORKER_H
