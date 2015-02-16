#include "intropage.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QtDebug>

IntroPage::IntroPage(QWidget *parent) :
    QWizardPage(parent)
{
    qDebug() << Q_FUNC_INFO << "Start";

    setTitle(tr("Network discovery tool for Jeedom boxes"));
    QPixmap watermark(":images/panel");
    setPixmap(QWizard::WatermarkPixmap, watermark);

    QLabel *label = new QLabel(tr(
        "This software will allow you to easily access your Jeedom boxes "
        "connected to your local network."
        "\n\nYour computer has to be on the same network than your Jeedom boxes."
        ));

//    label->setMaximumWidth(watermark.width()*1.5);
    label->setWordWrap(true);

    QRadioButton *normalSearch = new QRadioButton(tr("Use &automatic search"));
    normalSearch->setChecked(true);
    QRadioButton *advancedSearch = new QRadioButton(tr("Use ad&vanced search options"));
    registerField("advancedSearch", advancedSearch);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addStretch(2);
    layout->addWidget(normalSearch);
    layout->addWidget(advancedSearch);
    layout->addStretch(1);
    setLayout(layout);

    setButtonText(QWizard::NextButton, tr("Sea&rch"));

    qDebug() << Q_FUNC_INFO << "End";
}
