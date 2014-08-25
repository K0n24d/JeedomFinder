#ifndef PINGSEARCHWORKER_H
#define PINGSEARCHWORKER_H

#include "searchworker.h"
#include <QTimer>

class QProcess;

class PingSearchWorker : public SearchWorker
{
    Q_OBJECT
public:
    explicit PingSearchWorker(QObject *parent = 0);
    ~PingSearchWorker();

protected:
    bool stopping;
    QList<QProcess*> pingProcesses;
    QTimer checkResultsTimer;
#ifdef Q_OS_WIN
    QProcess arpTableProcess;
#endif
protected slots:
    void checkResults();
#ifdef Q_OS_WIN
    void gotArpResults(int);
#endif

public slots:
    void discover();
    void stop();
};

#endif // PINGSEARCHWORKER_H
