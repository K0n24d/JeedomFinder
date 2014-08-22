#ifndef SEARCHWORKER_H
#define SEARCHWORKER_H

#include <QObject>

class QProcess;

class SearchWorker : public QObject
{
    Q_OBJECT
public:
    explicit SearchWorker(QObject *parent = 0);
    ~SearchWorker();
protected:
    bool stopping;

    QList<QProcess*> pingProcesses;
signals:
    void searchPingFinished();
public slots:
    void sendPingRequests();
    void stop();
};

#endif // SEARCHWORKER_H
