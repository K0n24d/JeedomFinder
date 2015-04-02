#ifndef SEARCHWORKER_H
#define SEARCHWORKER_H

#include "host.h"
#include <QObject>
#include <QMutex>
#include <QTimer>

class QNetworkReply;
class QNetworkAccessManager;

class SearchWorker : public QObject
{
    Q_OBJECT
public:
    explicit SearchWorker(QObject *parent = 0);
    ~SearchWorker();

    static bool available();

    /*
    struct Host {
        QString name;
        QString ip;
        QString desc;
        QString url;
    };
    */

protected:
    void checkWebPage(const Host *host, QString url);
    QNetworkAccessManager *manager;
    bool allRequestsSent;
    int webPagesToCheck;
    QMutex webPagesToCheckMutex;
    QMutex checkWebPageMutex;
    QMutex replyFinishedMutex;
    bool stopping;
    QTimer checkWebPageTimer;

signals:
    void finished();
    void error(const QString & title, const QString & message);
    void host(Host * host);

public slots:
    virtual void discover(){};
    virtual void stop();

protected slots:
    void replyFinished(QNetworkReply* reply);
    void checkWebPageTimeout();
};


#endif // SEARCHWORKER_H
