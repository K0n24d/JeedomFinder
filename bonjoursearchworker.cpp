#include "bonjoursearchworker.h"
#include "bonjourservicebrowser.h"
#include "bonjourserviceresolver.h"

BonjourSearchWorker::BonjourSearchWorker(QObject *parent)
    : QObject(parent), bonjourResolver(0)
{
    connect(bonjourBrowser, SIGNAL(currentBonjourRecordsChanged(const QList<BonjourRecord> &)),
            this, SLOT(updateRecords(const QList<BonjourRecord> &)));
    bonjourBrowser->browseForServiceType(QLatin1String("_trollfortune._tcp"));
}

void BonjourSearchWorker::updateRecords(const QList<BonjourRecord> &list)
{
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
