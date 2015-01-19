#include "searchworker.h"
#include <QtDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMutexLocker>
#include <QThread>

SearchWorker::SearchWorker(QObject *parent) :
    QObject(parent), allRequestsSent(false), webPagesToCheck(0), checkWebPageTimer(this)
{
    manager=NULL;
    stopping=false;

    connect(&checkWebPageTimer, SIGNAL(timeout()), this, SLOT(checkWebPageTimeout()));
    checkWebPageTimer.setInterval(20000);
    checkWebPageTimer.setSingleShot(true);
}

SearchWorker::~SearchWorker()
{
    if(manager)
        delete manager;

    thread()->quit();
}

void SearchWorker::stop()
{
    qDebug() << Q_FUNC_INFO;

    stopping = true;
    emit(finished());
}

void SearchWorker::checkWebPage(const Host *host, QString url)
{
    if(stopping)
        return;

    QMutexLocker mutexLocker(&checkWebPageMutex);

    qDebug() << Q_FUNC_INFO << host->name << url;

    if(!manager)
    {
        manager = new QNetworkAccessManager(this);
        connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    }

    {
        QMutexLocker counterMutexLocker(&webPagesToCheckMutex);
        webPagesToCheck++;
    }

    Host * thisHost = new Host(host, this);
    if(thisHost->url.isEmpty())
        thisHost->url = url;
    QNetworkRequest request(url);

#ifdef HAS_SSL_SUPPORT
    // Autoriser les certificats auto-signés
    QSslConfiguration sslConfiguration = request.sslConfiguration();
    sslConfiguration.setPeerVerifyMode(QSslSocket::QueryPeer);
    request.setSslConfiguration(sslConfiguration);
#endif

    request.setOriginatingObject(thisHost);
    manager->get(request);
    qDebug() << Q_FUNC_INFO << "Démarrage timer";
    checkWebPageTimer.start();
}

void SearchWorker::replyFinished(QNetworkReply *reply)
{
    if(stopping)
        return;

    QMutexLocker mutexLocker(&replyFinishedMutex);

    {
        QMutexLocker counterMutexLocker(&webPagesToCheckMutex);
        webPagesToCheck--;
    }

    qDebug() << Q_FUNC_INFO << reply->url().toString() << "/" << reply->request().url().toString();

    reply->deleteLater();
    Host *thisHost = qobject_cast<Host*>(reply->request().originatingObject());

    if(!thisHost)
    {
        qDebug() << Q_FUNC_INFO << "originatingObject() incorrect.";
        return;
    }

    if (reply->error() == QNetworkReply::NoError)
    {
        QVariant redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        if(redirect.isValid())
        {
            QUrl newUrl(redirect.toUrl());
            if (newUrl.isRelative())
                newUrl = reply->request().url().resolved(newUrl);
            thisHost->desc.append(tr(" Redirection : %1").arg(newUrl.toString()));
            qDebug() << Q_FUNC_INFO <<  "Redirection vers" << newUrl.toString();
            checkWebPage(thisHost, newUrl.toString());
            thisHost->deleteLater();
            return;
        }

        QString page(reply->readAll());
        if(page.contains("<title>Jeedom</title>", Qt::CaseInsensitive))
            emit(host(thisHost));
    }
    else
    {
        qDebug() << Q_FUNC_INFO << reply->errorString();
        thisHost->deleteLater();
    }

    if(webPagesToCheck<=0 && allRequestsSent)
    {
        emit(finished());
    }
}

void SearchWorker::checkWebPageTimeout()
{
    qDebug() << Q_FUNC_INFO;

    if(allRequestsSent)
    {
        stop();
    }
}
