#include "advancedsearchpage.h"
#include <QGroupBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QVariant>
#include <QtDebug>

AdvancedSearchPage::AdvancedSearchPage(QWidget *parent) :
    QWizardPage(parent)
{
    qDebug() << Q_FUNC_INFO << "Start";

    setTitle(tr("Sélection des modes de recherche"));
    setSubTitle(tr("Veuillez sélectionner ci-dessous les options utilisées lors de la recherche"));

    QGroupBox *modesRecherche = new QGroupBox(tr("Modes de recherche :"));

    QCheckBox *zeroconf = new QCheckBox(tr("Recherche via Zeroconf"));
    zeroconf->setChecked(true);
    registerField("zeroconf", zeroconf);
    connect(zeroconf, SIGNAL(stateChanged(int)), SIGNAL(completeChanged()));

    QCheckBox *udp = new QCheckBox(tr("Recherche via UDP + ARP Cache"));
    udp->setChecked(true);
    registerField("udp", udp);
    connect(udp, SIGNAL(stateChanged(int)), SIGNAL(completeChanged()));

    QCheckBox *ping = new QCheckBox(tr("Recherche via Ping + ARP Cache"));
    ping->setChecked(false);
    registerField("ping", ping);
    connect(ping, SIGNAL(stateChanged(int)), SIGNAL(completeChanged()));

    QCheckBox *dns = new QCheckBox(tr("Recherche via DNS"));
    dns->setChecked(true);
    registerField("dns", dns);
    connect(dns, SIGNAL(stateChanged(int)), SIGNAL(completeChanged()));

    arpscan = new QCheckBox(tr("Recherche via ARP Ping"));
    registerField("arpscan", arpscan);
    connect(arpscan, SIGNAL(stateChanged(int)), SIGNAL(completeChanged()));

    QVBoxLayout *modesRechercheLayout = new QVBoxLayout();
    modesRechercheLayout->addWidget(zeroconf);
    modesRechercheLayout->addWidget(udp);
    modesRechercheLayout->addWidget(ping);
    modesRechercheLayout->addWidget(dns);
    modesRechercheLayout->addWidget(arpscan);
    modesRecherche->setLayout(modesRechercheLayout);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(modesRecherche);
    layout->addStretch(1);
    setLayout(layout);

    qDebug() << Q_FUNC_INFO << "End";
}

void AdvancedSearchPage::initializePage()
{
    qDebug() << Q_FUNC_INFO << "Start";

    if(!parent()->property("hasAdminRights").toBool())
    {
        arpscan->setDisabled(true);
        arpscan->setChecked(false);
    }
    else
        arpscan->setChecked(true);

    qDebug() << Q_FUNC_INFO << "End";
}

bool AdvancedSearchPage::isComplete() const
{
    if(field("zeroconf").toBool()
            || field("udp").toBool()
            || field("ping").toBool()
            || field("dns").toBool()
            || field("arpscan").toBool()
            )
        return true;

    return false;
}
