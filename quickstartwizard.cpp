#include "quickstartwizard.h"
#include "intropage.h"
#include "searchpage.h"

QuickStartWizard::QuickStartWizard(QWidget *parent) :
    QWizard(parent)
{
    setPage(Page_Intro, new IntroPage);
    setPixmap(QWizard::LogoPixmap, QPixmap(":/images/logo"));
}
