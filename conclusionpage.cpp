#include "conclusionpage.h"
#include <QtDebug>

ConclusionPage::ConclusionPage(QWidget *parent) :
    QWizardPage(parent)
{
    qDebug() << Q_FUNC_INFO << "Start";

    setTitle(tr("Recherche du serveur Jeedom"));
    setSubTitle(tr("Cet utilitaire vous guiderez dans le démarrage avec votre serveur Jeedom"));

    qDebug() << Q_FUNC_INFO << "End";
}
