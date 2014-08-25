#include "conclusionpage.h"

ConclusionPage::ConclusionPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Recherche du serveur Jeedom"));
    setSubTitle(tr("Cet utilitaire vous guiderez dans le démarrage avec votre serveur Jeedom"));
}
