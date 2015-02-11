#include "intropage.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QtDebug>

IntroPage::IntroPage(QWidget *parent) :
    QWizardPage(parent)
{
    qDebug() << Q_FUNC_INFO << "Start";

    setTitle(tr("Logiciel de découverte réseau de box Jeedom"));
    QPixmap watermark(":images/panel");
    setPixmap(QWizard::WatermarkPixmap, watermark);

    QLabel *label = new QLabel(tr(
        "Ce logiciel va vous permettre d'accéder facilement à votre box domotique "
        "Jeedom qui est connectée sur votre réseau local."
        "\n\nVotre ordinateur doit être sur le même réseau que votre box Jeedom."
        ));

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

    setButtonText(QWizard::NextButton, tr("Lancer la &recherche"));

    qDebug() << Q_FUNC_INFO << "End";
}
