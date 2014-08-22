#include "advancedsearchpage.h"
#include <QGroupBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QVariant>

AdvancedSearchPage::AdvancedSearchPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Sélection des modes de recherche"));
    setSubTitle(tr("Veuillez sélectionner ci-dessous les options utilisées lors de la recherche"));

    QGroupBox *modesRecherche = new QGroupBox(tr("Modes de recherche :"));
    QCheckBox *zeroconf = new QCheckBox(tr("Recherche via Zeroconf"));
    zeroconf->setChecked(true);
    registerField("zeroconf", zeroconf);
    connect(zeroconf, SIGNAL(stateChanged(int)), SIGNAL(completeChanged()));

    arpscan = new QCheckBox(tr("Recherche via ARP Ping"));
    registerField("arpscan", arpscan);
    connect(arpscan, SIGNAL(stateChanged(int)), SIGNAL(completeChanged()));

    QVBoxLayout *modesRechercheLayout = new QVBoxLayout();
    modesRechercheLayout->addWidget(zeroconf);
    modesRechercheLayout->addWidget(arpscan);
    modesRecherche->setLayout(modesRechercheLayout);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(modesRecherche);
    layout->addStretch(1);
    setLayout(layout);

}

void AdvancedSearchPage::initializePage()
{
    if(!parent()->property("hasAdminRights").toBool())
    {
        arpscan->setDisabled(true);
        arpscan->setChecked(false);
    }
    else
        arpscan->setChecked(true);
}

bool AdvancedSearchPage::isComplete() const
{
    if(!field("zeroconf").toBool() && !field("arpscan").toBool())
        return false;

    return true;
}
