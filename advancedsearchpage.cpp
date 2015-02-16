#include "advancedsearchpage.h"
#include <QGroupBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QVariant>
#include <QtDebug>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>

AdvancedSearchPage::AdvancedSearchPage(QWidget *parent) :
    QWizardPage(parent)
{
    qDebug() << Q_FUNC_INFO << "Start";

    setTitle(tr("Search mode setup"));
    setSubTitle(tr("Please select the options to be applied during the search"));

    QGroupBox *modesRecherche = new QGroupBox(tr("Search modes :"));

    QCheckBox *zeroconf = new QCheckBox(tr("Search using Zeroconf"));
    zeroconf->setChecked(true);
    registerField("zeroconf", zeroconf);
    connect(zeroconf, SIGNAL(stateChanged(int)), SIGNAL(completeChanged()));

    udp = new QCheckBox(tr("Search using UDP + ARP Cache"));
    udp->setChecked(true);
    registerField("udp", udp);
    connect(udp, SIGNAL(stateChanged(int)), SIGNAL(completeChanged()));
    connect(udp, SIGNAL(stateChanged(int)), SLOT(udpChanged()));

    QGroupBox *optionsRecherche = new QGroupBox(tr("Networks to be scanned with UDP + ARP Cache :"));
    QVBoxLayout *optionsRechercheLayout = new QVBoxLayout();
    optionsRecherche->setLayout(optionsRechercheLayout);
    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
    {
        if(   !interface.flags().testFlag(QNetworkInterface::IsUp)
           || interface.flags().testFlag(QNetworkInterface::IsLoopBack)
           || interface.flags().testFlag(QNetworkInterface::IsPointToPoint)
              )
            continue;

        foreach(QNetworkAddressEntry addressEntry, interface.addressEntries())
        {
            if(addressEntry.ip().protocol()!=QAbstractSocket::IPv4Protocol)
                continue;
            QCheckBox *ip = new QCheckBox(tr("Network : %1/%2").arg(addressEntry.ip().toString(), addressEntry.netmask().toString()));
            ip->setChecked(true);
            ipCheckBoxes << ip;
            registerField(addressEntry.ip().toString(), ip);
            optionsRechercheLayout->addWidget(ip);
            connect(ip, SIGNAL(clicked()), SLOT(ipAddressesChanged()));
        }
    }

    QCheckBox *ping = new QCheckBox(tr("Search using Ping + ARP Cache"));
    ping->setChecked(false);
    registerField("ping", ping);
    connect(ping, SIGNAL(stateChanged(int)), SIGNAL(completeChanged()));

    QCheckBox *dns = new QCheckBox(tr("Search using DNS"));
    dns->setChecked(true);
    registerField("dns", dns);
    connect(dns, SIGNAL(stateChanged(int)), SIGNAL(completeChanged()));

    arpscan = new QCheckBox(tr("Search using ARP Ping"));
    registerField("arpscan", arpscan);
    connect(arpscan, SIGNAL(stateChanged(int)), SIGNAL(completeChanged()));

    QVBoxLayout *modesRechercheLayout = new QVBoxLayout();
    modesRechercheLayout->addWidget(zeroconf);
    modesRechercheLayout->addWidget(udp);
    modesRechercheLayout->addWidget(ping);
    modesRechercheLayout->addWidget(dns);
    modesRechercheLayout->addWidget(arpscan);
    modesRecherche->setLayout(modesRechercheLayout);

    QVBoxLayout *vglayout = new QVBoxLayout;
    vglayout->addWidget(modesRecherche);
    vglayout->addStretch(1);

    QVBoxLayout *vdlayout = new QVBoxLayout;
    vdlayout->addWidget(optionsRecherche);
    vdlayout->addStretch(1);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addLayout(vglayout);
    hlayout->addLayout(vdlayout);
    setLayout(hlayout);

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

void AdvancedSearchPage::ipAddressesChanged()
{
    foreach(QCheckBox *checkBox, ipCheckBoxes)
    {
        if(checkBox->isChecked())
        {
            udp->setChecked(true);
            return;
        }
    }
    udp->setChecked(false);
}

void AdvancedSearchPage::udpChanged()
{
    foreach(QCheckBox *checkBox, ipCheckBoxes)
    {
        checkBox->setChecked(udp->isChecked());
    }
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
