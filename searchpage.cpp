#include "searchpage.h"
#include "pingsearchworker.h"
#include <QVariant>
#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>
#include <QHostInfo>
#include <QMessageBox>

SearchPage::SearchPage(QWidget *parent) :
    QWizardPage(parent)
{
    numberOfSearchWorkersRunning=0;

    setTitle(tr("Recherche et sélection du serveur Jeedom"));

#ifdef Q_OS_WIN
    connect(&arpTableProcess, SIGNAL(finished(int)), this, SLOT(gotArpResults(int)));
#endif

    checkResultsTimer.setInterval(1000);
    checkResultsTimer.setSingleShot(true);

    connect(&checkResultsTimer, SIGNAL(timeout()), this, SLOT(checkResults()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addStretch();
    layout->addWidget(&progressBar);
    setLayout(layout);
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

    numberOfSearchWorkersRunning=0;

    if(field("ping").toBool())
    {
        PingSearchWorker *worker = new PingSearchWorker;
        worker->moveToThread(&searchThread);
        connect(worker, SIGNAL(searchPingFinished()), this, SLOT(searchFinished()));
        connect(&searchThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
        connect(&searchThread, SIGNAL(started()), worker, SLOT(sendPingRequests()));
        connect(this, SIGNAL(cleaningUp()), worker, SLOT(stop()));
        numberOfSearchWorkersRunning++;
    }

    if(numberOfSearchWorkersRunning>0)
    {
        progressBar.setRange(0,0);
        progressBar.setVisible(true);
    }
    else
    {
        progressBar.setRange(0,100);
        progressBar.setVisible(false);
    }


    searchThread.start();
    checkResultsTimer.start();
}

void SearchPage::cleanupPage()
{
    emit(cleaningUp());
#ifdef Q_OS_WIN
    if(arpTableProcess.state()!=QProcess::NotRunning)
        arpTableProcess.kill();
#endif
    checkResultsTimer.stop();
    searchThread.quit();
}

bool SearchPage::isComplete() const
{
    return false;
}

#ifdef Q_OS_WIN
void SearchPage::checkResults()
{
    if(arpTableProcess.state()!=QProcess::NotRunning)
    {
        if(numberOfSearchWorkersRunning>0)
            checkResultsTimer.start();
        return;
    }

    arpTableProcess.start("arp", QStringList() << "-a");
}

void SearchPage::gotArpResults(int)
{
    QRegExp rx("^ *([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}) *([^ -]+-[^ -]+-[^ -]+-[^ -]+-[^ -]+-[^ -]+).*$");
            /*
    QRegExp rx(" *\\([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\)"
               " +\\([^ ]+\\)"
               " .*");
                       */
    while(!arpTableProcess.atEnd())
    {
        QString line(arpTableProcess.readLine());
        int pos = rx.indexIn(line);
        if(pos<0)
            continue;

        QStringList list = rx.capturedTexts();
        if(list.count()!=3)
            continue;

        QString mac = list.at(2).split('-').join(':').toUpper();
        if(mac.startsWith("B8:27:EB") || mac.startsWith("52:54:00"))
        {
            emit(hostFound(list.at(1), mac));
        }
//        QHostInfo hostInfo(QHostInfo::fromName(address));
//        QString name = hostInfo.hostName();
    }

    checkResultsTimer.start();
}
#endif

#ifdef Q_OS_LINUX
void SearchPage::checkResults()
{
    QFile arpTable("/proc/net/arp");

    if(arpTable.open(QFile::ReadOnly))
    {
        QRegExp rx("^ *([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})[^:]*([^ :]+:[^ :]+:[^ :]+:[^ :]+:[^ :]+:[^ :]+).*$");
        QByteArray data = arpTable.readAll();
        QTextStream in(&data, QIODevice::ReadOnly);

        while(!in.atEnd())
        {
            QString line(in.readLine());
            int pos = rx.indexIn(line);

            if(pos<0)
                continue;

            QStringList list = rx.capturedTexts();
            if(list.count()!=3)
                continue;

            QString mac = list.at(2).toUpper();
            if(!mac.compare("00:00:00:00:00:00"))
                continue;

            if(mac.startsWith("B8:27:EB") || mac.startsWith("52:54:00") || mac.startsWith("42:98:42"))
            {
                emit(hostFound(list.at(1), mac));
            }
        }
    }
    else
        QMessageBox::warning(NULL, Q_FUNC_INFO, "Could not open /proc/net/arp", QMessageBox::Close);


    if(numberOfSearchWorkersRunning>0)
        checkResultsTimer.start();
}
#endif

void SearchPage::searchFinished()
{
    numberOfSearchWorkersRunning--;

    if(numberOfSearchWorkersRunning<=0)
    {
        progressBar.setRange(0,100);
        progressBar.setVisible(false);
    }
}
