#include "searchpage.h"
#include "searchthread.h"
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

    searchThread = new SearchThread();
    searchThread->start();

    checkResultsTimer.start();
}

void SearchPage::cleanupPage()
{
    checkResultsTimer.stop();
    searchThread->stop();
    if(!searchThread->wait(1000))
        searchThread->terminate();
    delete searchThread;
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
