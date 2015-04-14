#include "intropage.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QtDebug>
#include <QMessageBox>

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

    {
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addStretch(1);
    QLabel *aboutLabel = new QLabel(tr("<html><body><a href=\"about\">About</a></body></html>"));
    aboutLabel->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
    connect(aboutLabel, SIGNAL(linkActivated(QString)), this, SLOT(linkActivated(QString)));
    hLayout->addWidget(aboutLabel);
    layout->addLayout(hLayout);
    }

    setLayout(layout);

    setButtonText(QWizard::NextButton, tr("Sea&rch"));

    qDebug() << Q_FUNC_INFO << "End";
}

void IntroPage::linkActivated(const QString & link)
{
    if(link.compare("about", Qt::CaseInsensitive)==0)
    {
//        tr("<html><body><a href=\"aboutQt\">About</a></body></html>"));
//        aboutLabel->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
//        connect(aboutLabel, SIGNAL(linkActivated(QString)), this, SLOT(onLinkActivated(QString)));

        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("About JeedomFinder"));
//        msgBox.setTextFormat(Qt::RichText);   //this is what makes the links clickable
        msgBox.setIconPixmap(parentWidget()->windowIcon().pixmap(64,64));
        msgBox.setText(tr("JeedomFinder %1").arg(GIT_VERSION));
        msgBox.setInformativeText(tr(
            "<html><body>"
            "<p>JeedomFinder is released under <a href=\"http://www.gnu.org/licenses/gpl.html\">GPLv3</a>.<br>"
            "The source code can be found in <a href=\"https://github.com/K0n24d/JeedomFinder\">this</a> Github repository.</p>"
            "<p>This software uses the Qt Libraries.<br>For more information see <b>About Qt</b> below.</p>"
            "</body></html>"
            ));
        msgBox.addButton(QMessageBox::Close);
        QPushButton *aboutQtButton = msgBox.addButton(tr("About Qt"), QMessageBox::ActionRole);
        msgBox.exec();
        if((QPushButton *)msgBox.clickedButton() == aboutQtButton)
            QMessageBox::aboutQt(parentWidget());
    }
}
