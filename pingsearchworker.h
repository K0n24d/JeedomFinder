#ifndef PINGSEARCHWORKER_H
#define PINGSEARCHWORKER_H

#include "searchworker.h"
#include <QTimer>

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
#include <QProcess>
#else
class QProcess;
#endif

#include <QNetworkAccessManager>

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
    QNetworkAccessManager *manager;
    QList<QString> checkedMACs;
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    QProcess *arpTableProcess;
#endif
    void checkWebPage(const Host *host, QString url);

protected slots:
    void checkResults();
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    void gotArpResults(int);
#endif
    void replyFinished(QNetworkReply* reply);

public slots:
    void discover();
    void stop();
};

#endif // PINGSEARCHWORKER_H
