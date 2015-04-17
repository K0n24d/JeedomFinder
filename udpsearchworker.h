#ifndef UDPSEARCHWORKER_H
#define UDPSEARCHWORKER_H

#include "searchworker.h"
#include <QTimer>
#include <QUdpSocket>
#include <QHostInfo>

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
#include <QProcess>
#include <QMutex>
#else
class QProcess;
#endif

class QWizard;

class UdpSearchWorker : public SearchWorker
{
    Q_OBJECT
public:
    explicit UdpSearchWorker(QWizard *pWizard, QObject *parent = 0);
    ~UdpSearchWorker();

protected:
    QList<QUdpSocket*> sockets;
    QTimer *checkResultsTimer;
    QList<QString> checkedMACs;
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    QProcess *arpTableProcess;
    QMutex arpMutex;
#endif
    QHash<int, QString> lookupIDs;
    QWizard *wizard;

    bool hasFinished();
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

#endif // UDPSEARCHWORKER_H
