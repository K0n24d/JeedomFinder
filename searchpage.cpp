#include "searchpage.h"

SearchPage::SearchPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Recherche et sélection du serveur Jeedom"));
    setSubTitle(tr("Cet utilitaire vous guiderez dans le démarrage avec votre serveur Jeedom"));
}
