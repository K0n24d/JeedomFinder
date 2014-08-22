#include "searchpage.h"
#include "searchworker.h"
#include <QVariant>
#include <QFile>
#include <QTextStream>

SearchPage::SearchPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Recherche et sélection du serveur Jeedom"));
    checkResultsTimer.setInterval(1000);

    connect(&checkResultsTimer, SIGNAL(timeout()), this, SLOT(checkResults()));
}

void SearchPage::initializePage()
{
    QString subTitle;

    if(!field("advancedSearch").toBool())
    {
        subTitle=tr("Recherche automatique en cours...");
    }
    else
    {
        subTitle=tr("Recherche avancée en cours...");
    }

    setSubTitle(subTitle);

    SearchWorker *worker = new SearchWorker;
    worker->moveToThread(&searchThread);
    connect(&searchThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(&searchThread, SIGNAL(started()), worker, SLOT(sendPingRequests()));
    connect(this, SIGNAL(cleaningUp()), worker, SLOT(stop()));
    searchThread.start();

    checkResultsTimer.start();
}

void SearchPage::cleanupPage()
{
    emit(cleaningUp());
    checkResultsTimer.stop();
    searchThread.quit();
}

bool SearchPage::isComplete() const
{
    return false;
}

void SearchPage::checkResults()
{
    QFile arpTable("/proc/net/arp");

    if(arpTable.open(QFile::ReadOnly))
    {
        QTextStream in(&arpTable);
        while(!in.atEnd())
        {
            QString line(in.readLine());
        }
    }
}
