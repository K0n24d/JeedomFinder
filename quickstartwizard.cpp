#include "quickstartwizard.h"
#include "intropage.h"
#include "advancedsearchpage.h"
#include "searchpage.h"
#include "conclusionpage.h"
#include <QVariant>
#include <QIcon>
#include <QtDebug>

#if defined(Q_OS_UNIX)
#include <unistd.h>
#endif

QuickStartWizard::QuickStartWizard(QWidget *parent) :
    QWizard(parent)
{
    qDebug() << Q_FUNC_INFO << "Start";

#if defined(Q_OS_UNIX)
    setProperty("hasAdminRights", !geteuid());
#elif defined(Q_OS_WIN)
    setProperty("hasAdminRights", true);
#else
    setProperty("hasAdminRights", false);
#endif

    setWindowTitle(tr("Jeedom"));
    setWindowIcon(QIcon(":/images/icon"));

    setWizardStyle(QWizard::ModernStyle);

    setPage(Page_Intro, new IntroPage(this));
    setPage(Page_AdvancedSearch, new AdvancedSearchPage(this));
    setPage(Page_Search, new SearchPage(this));
    setPage(Page_Conclusion, new ConclusionPage(this));

    setPixmap(QWizard::LogoPixmap, QPixmap(":/images/logo"));

    qDebug() << Q_FUNC_INFO << "End";
}

int QuickStartWizard::nextId() const
{
    switch (currentId()) {
    case Page_Intro:
        if (field("advancedSearch").toBool()) {
            return Page_AdvancedSearch;
        } else {
            return Page_Search;
        }
    case Page_AdvancedSearch:
        return Page_Search;
    case Page_Search:
        return Page_Conclusion;
    case Page_Conclusion:
    default:
        return -1;
    }
}
