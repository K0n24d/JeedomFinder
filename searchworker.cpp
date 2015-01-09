#include "searchworker.h"
#include <QtDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMutexLocker>

SearchWorker::SearchWorker(QObject *parent) :
    QObject(parent), allRequestsSent(false), webPagesToCheck(0)
{
    manager=NULL;
}

SearchWorker::~SearchWorker()
{
    if(manager)
        manager->deleteLater();
}

void SearchWorker::checkWebPage(const Host *host, QString url)
{
    qDebug() << Q_FUNC_INFO << host->name << url;

    if(!manager)
    {
        manager = new QNetworkAccessManager(this);
        connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    }

    {
        QMutexLocker mutexLocker(&webPagesToCheckMutex);
        webPagesToCheck++;
    }

    Host * thisHost = new Host(host, this);
    if(thisHost->url.isEmpty())
        thisHost->url = url;
    QNetworkRequest request(url);
    request.setOriginatingObject(thisHost);
    manager->get(request);
}

void SearchWorker::replyFinished(QNetworkReply *reply)
{
    qDebug() << Q_FUNC_INFO;

    {
        QMutexLocker mutexLocker(&webPagesToCheckMutex);
        webPagesToCheck--;
    }

    reply->deleteLater();
    Host *thisHost = static_cast<Host*>(reply->request().originatingObject());
    if(!thisHost)
        return;

    if (reply->error() == QNetworkReply::NoError)
    {
        QVariant redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        if(redirect.isValid())
        {
            QUrl newUrl(redirect.toUrl());
            if (newUrl.isRelative())
                newUrl = reply->request().url().resolved(newUrl);
            thisHost->desc.append(tr(" Redirect : %1").arg(newUrl.toString()));
            checkWebPage(thisHost, newUrl.toString());
            thisHost->deleteLater();
            return;
        }
        QString page(reply->readAll());
        if(page.contains("<title>Jeedom</title>", Qt::CaseInsensitive))
            emit(host(thisHost));
    }

    if(webPagesToCheck<=0 && allRequestsSent)
    {
        emit(finished());
    }
}
