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
    void finished();
    void error(const QString, const QString);
public slots:
    void discover();
    void stop();
};

#endif // PINGSEARCHWORKER_H
