#include "searchpage.h"
#include "pingsearchworker.h"
#include "bonjoursearchworker.h"
#include <QVariant>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTableWidget>

SearchPage::SearchPage(QWidget *parent) :
    QWizardPage(parent)
{
    qRegisterMetaType<SearchWorker::Host>("SearchWorker::Host");

    numberOfSearchWorkersRunning=0;

    setTitle(tr("Recherche et sélection du serveur Jeedom"));

#ifdef Q_OS_WIN
    connect(&arpTableProcess, SIGNAL(finished(int)), this, SLOT(gotArpResults(int)));
#endif

    hostsTable.setColumnCount(4);
    hostsTable.setRowCount(0);
    hostsTable.setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList labels;
    labels << tr("Nom") << tr("URL") << tr("Adresse IP") << tr("Description");
    hostsTable.setHorizontalHeaderLabels(labels);
    hostsTable.setDisabled(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(&hostsTable);
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

    if(field("zeroconf").toBool())
        addWorker(new BonjourSearchWorker);

    if(field("ping").toBool())
        addWorker(new PingSearchWorker);

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
}

void SearchPage::addWorker(SearchWorker *worker)
{
    worker->moveToThread(&searchThread);
    connect(worker, SIGNAL(finished()), this, SLOT(searchFinished()));
    connect(worker, SIGNAL(error(QString,QString)), this, SLOT(gotError(QString,QString)));

    connect(worker, SIGNAL(host(SearchWorker::Host)), this, SLOT(gotHost(SearchWorker::Host)));
    connect(&searchThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(&searchThread, SIGNAL(started()), worker, SLOT(discover()));
    connect(this, SIGNAL(cleaningUp()), worker, SLOT(stop()));
    numberOfSearchWorkersRunning++;
}

void SearchPage::cleanupPage()
{
    emit(cleaningUp());
#ifdef Q_OS_WIN
    if(arpTableProcess.state()!=QProcess::NotRunning)
        arpTableProcess.kill();
#endif
    searchThread.quit();
}

bool SearchPage::isComplete() const
{
    return false;
}

void SearchPage::gotHost(const SearchWorker::Host &host)
{
//    QMessageBox::information(NULL, Q_FUNC_INFO, tr("Nom: %1, IPv4: %2, Description: %3").arg(name, ipv4, description));
    if(!hostsTable.findItems(host.url, Qt::MatchExactly).isEmpty())
        return;

    hostsTable.setEnabled(true);

    int row=hostsTable.rowCount();
    hostsTable.setRowCount(row+1);

    QTableWidgetItem * name = new QTableWidgetItem(host.name);
    name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    hostsTable.setItem(row, 0, name);

    QTableWidgetItem * url = new QTableWidgetItem(host.url);
    name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    hostsTable.setItem(row, 1, url);

    QTableWidgetItem * ip = new QTableWidgetItem(host.ip);
    name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    hostsTable.setItem(row, 2, ip);

    QTableWidgetItem * desc = new QTableWidgetItem(host.desc);
    name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    hostsTable.setItem(row, 3, desc);

    hostsTable.resizeColumnsToContents();
}

void SearchPage::gotError(const QString &title, const QString &message)
{
    QMessageBox::warning(NULL, title, message, QMessageBox::Close);
}

void SearchPage::searchFinished()
{
    numberOfSearchWorkersRunning--;

    if(numberOfSearchWorkersRunning<=0)
    {
        progressBar.setRange(0,100);
        progressBar.setVisible(false);
    }
}
