#include "intropage.h"

IntroPage::IntroPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Démarrage rapide Jeedom"));
    setSubTitle(tr("Cet utilitaire vous guiderez dans le démarrage avec votre serveur Jeedom"));
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":images/panel"));
}
