#include "bonjoursearchworker.h"
#include "bonjourservicebrowser.h"
#include "bonjourserviceresolver.h"

BonjourSearchWorker::BonjourSearchWorker(QObject *parent)
    : QObject(parent)
{
    bonjourBrowser = new BonjourServiceBrowser(this);
    bonjourResolver = new BonjourServiceResolver(this);

    connect(bonjourBrowser, SIGNAL(currentBonjourRecordsChanged(const QList<BonjourRecord> &)),
            this, SLOT(updateRecords(const QList<BonjourRecord> &)));
}

void BonjourSearchWorker::discover()
{
    bonjourBrowser->browseForServiceType(QString("_printer._tcp"));
}

void BonjourSearchWorker::stop()
{

}

void BonjourSearchWorker::updateRecords(const QList<BonjourRecord> &list)
{
    foreach (BonjourRecord record, list)
    {
        emit(error(Q_FUNC_INFO, record.serviceName));
    }
    /*
    treeWidget->clear();
    foreach (BonjourRecord record, list) {
        QVariant variant;
        variant.setValue(record);
        QTreeWidgetItem *processItem = new QTreeWidgetItem(treeWidget,
                                                           QStringList() << record.serviceName);
        processItem->setData(0, Qt::UserRole, variant);
    }
    
    if (treeWidget->invisibleRootItem()->childCount() > 0) {
        treeWidget->invisibleRootItem()->child(0)->setSelected(true);
    }
    enableGetFortuneButton();
    */
}
