#include "advancedsearchpage.h"

AdvancedSearchPage::AdvancedSearchPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Sélection des modes de recherche"));
    setSubTitle(tr("Cet utilitaire vous guiderez dans le démarrage avec votre serveur Jeedom"));
}
