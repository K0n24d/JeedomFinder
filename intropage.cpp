#include "intropage.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QRadioButton>

IntroPage::IntroPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Démarrage rapide Jeedom"));
    QPixmap watermark(":images/panel");
    setPixmap(QWizard::WatermarkPixmap, watermark);

    QLabel *label = new QLabel(tr(
        "JEEDOM est un projet domotique qui a pour but de connecter sa maison, "
        "ses appareils électroniques, ses projets domotiques, tous ensembles."
        "\n\nA distance, en local, par la voix, les sms, ou encore le tactile, "
        "vous pouvez piloter votre maison à tout instant et être alerté d'un événement."
        "\n\nCet utilitaire vous guiderez dans le démarrage avec votre serveur Jeedom"));

//    label->setMaximumWidth(watermark.width()*1.5);
    label->setWordWrap(true);

    QRadioButton *normalSearch = new QRadioButton(tr("Utiliser la recherche &automatique"));
    normalSearch->setChecked(true);
    QRadioButton *advancedSearch = new QRadioButton(tr("Utiliser la recherche a&vancée"));
    registerField("advancedSearch", advancedSearch);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addStretch(2);
    layout->addWidget(normalSearch);
    layout->addWidget(advancedSearch);
    layout->addStretch(1);
    setLayout(layout);
}
