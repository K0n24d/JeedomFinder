#ifndef PINGSEARCHWORKER_H
#define PINGSEARCHWORKER_H

#include <QObject>

class QProcess;

class PingSearchWorker : public QObject
{
    Q_OBJECT
public:
    explicit PingSearchWorker(QObject *parent = 0);
    ~PingSearchWorker();
protected:
    bool stopping;

    QList<QProcess*> pingProcesses;
signals:
    void searchPingFinished();
public slots:
    void sendPingRequests();
    void stop();
};

#endif // PINGSEARCHWORKER_H
