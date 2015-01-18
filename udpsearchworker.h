#ifndef UDPSEARCHWORKER_H
#define UDPSEARCHWORKER_H

#include "searchworker.h"
#include <QTimer>
#include <QUdpSocket>

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
#include <QProcess>
#include <QMutex>
#else
class QProcess;
#endif

class UdpSearchWorker : public SearchWorker
{
    Q_OBJECT
public:
    explicit UdpSearchWorker(QObject *parent = 0);
    ~UdpSearchWorker();

protected:
    bool stopping;
    QTimer *checkResultsTimer;
    QList<QString> checkedMACs;
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    QProcess *arpTableProcess;
    QMutex arpMutex;
#endif
    QUdpSocket socket;

protected slots:
    void checkResults();
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    void gotArpResults(int);
#endif

public slots:
    void discover();
    void stop();
};

#endif // UDPSEARCHWORKER_H
