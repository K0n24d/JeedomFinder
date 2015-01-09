#include "searchpage.h"
#include "pingsearchworker.h"
#include "bonjoursearchworker.h"
#include "dnslookupsearchworker.h"
#include <QVariant>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTableWidget>
#include <QLabel>
#include <QHeaderView>
#include <QtDebug>

SearchPage::SearchPage(QWidget *parent) :
    QWizardPage(parent), progressBar(this), hostsTable(this)
{
    qDebug() << Q_FUNC_INFO << "Start";
//    qRegisterMetaType<Host>("Host");

    progressBar.setTextVisible(false);

    setTitle(tr("Recherche et sélection du serveur Jeedom"));

    hostsTable.setColumnCount(3);
    hostsTable.setRowCount(0);
    hostsTable.setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList labels;
    labels << tr("Nom") << tr("URL") << tr("Adresse IP") << tr("Description");
    hostsTable.setHorizontalHeaderLabels(labels);
    hostsTable.setDisabled(true);
    connect(&hostsTable, SIGNAL(itemSelectionChanged()), this, SIGNAL(completeChanged()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(&hostsTable);
    layout->addStretch();
    layout->addWidget(&progressBar);
    setLayout(layout);

    qDebug() << Q_FUNC_INFO << "End";
}

void SearchPage::initializePage()
{
    qDebug() << Q_FUNC_INFO;

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

    searchWorkers.empty();

    if(field("dns").toBool())
        addWorker(new DNSLookupSearchWorker);

    if(field("zeroconf").toBool())
    {
        addWorker(new BonjourSearchWorker(QString("_https._tcp")));
        addWorker(new BonjourSearchWorker(QString("_http._tcp")));
    }

    if(field("ping").toBool())
        addWorker(new PingSearchWorker);

    if(!searchWorkers.isEmpty())
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
    qDebug() << Q_FUNC_INFO;

    worker->moveToThread(&searchThread);
    connect(worker, SIGNAL(finished()), this, SLOT(searchFinished()));
    connect(worker, SIGNAL(error(QString,QString)), this, SLOT(gotError(QString,QString)));

    connect(worker, SIGNAL(host(Host*)), this, SLOT(gotHost(Host*)));
    connect(&searchThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(&searchThread, SIGNAL(started()), worker, SLOT(discover()));
    connect(this, SIGNAL(cleaningUp()), worker, SLOT(stop()));

    searchWorkers << worker;
}

void SearchPage::resizeEvent(QResizeEvent *)
{
    qDebug() << Q_FUNC_INFO;

    hostsTable.setColumnWidth(0, hostsTable.width()/4);
    hostsTable.setColumnWidth(2, hostsTable.width()/4);

#if QT_VERSION >= 0x050000
    hostsTable.horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
#else
    hostsTable.horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
#endif
}

void SearchPage::cleanupPage()
{
    emit(cleaningUp());
    searchThread.quit();
}

bool SearchPage::isComplete() const
{
    return !(hostsTable.selectedRanges().isEmpty());
}

void SearchPage::gotHost(Host *host)
{
    qDebug() << Q_FUNC_INFO << host;

//    QMessageBox::information(NULL, Q_FUNC_INFO, tr("Nom: %1, IPv4: %2, Description: %3").arg(name, ipv4, description));


    if(!hostsTable.findItems(host->url, Qt::MatchExactly).isEmpty())
    {
        host->deleteLater();
        return;
    }

    hostsTable.setEnabled(true);

    int row=hostsTable.rowCount();
    hostsTable.setRowCount(row+1);

    QTableWidgetItem * item = new QTableWidgetItem(host->name);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setToolTip(host->desc);
    hostsTable.setItem(row, 0, item);

    QLabel *label = new QLabel(QString("<html><body><a href=\"%1\">%2</a></body></html>").arg(host->url, host->url));
    label->setOpenExternalLinks(true);
    label->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
    label->setToolTip(host->desc);
    hostsTable.setCellWidget(row, 1, label);

    item = new QTableWidgetItem();
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setToolTip(host->desc);
    hostsTable.setItem(row, 1, item);

    item = new QTableWidgetItem(host->ip);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setToolTip(host->desc);
    hostsTable.setItem(row, 2, item);

    host->deleteLater();
}

void SearchPage::gotError(const QString &title, const QString &message)
{
    qDebug() << Q_FUNC_INFO << title << message;

    QMessageBox::warning(NULL, title, message, QMessageBox::Close);
}

void SearchPage::searchFinished()
{
    bool workerConsideredRunning = searchWorkers.contains(sender());
    qDebug() << Q_FUNC_INFO << searchWorkers.count() << workerConsideredRunning;

    if(!workerConsideredRunning)
        return;

    searchWorkers.removeOne(sender());

    if(searchWorkers.isEmpty())
    {
        progressBar.setRange(0,100);
        progressBar.setVisible(false);
        if(hostsTable.rowCount()<=0)
            QMessageBox::warning(this, tr("Jeedom"), tr("Aucun serveur Jeedom n'a pu être trouvé"), QMessageBox::Close, QMessageBox::Close);
    }
}
