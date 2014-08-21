#include "quickstartwizard.h"
#include "intropage.h"
#include "advancedsearchpage.h"
#include "searchpage.h"
#include "conclusionpage.h"
#include <QVariant>

QuickStartWizard::QuickStartWizard(QWidget *parent) :
    QWizard(parent)
{
    setWindowTitle(tr("Jeedom"));
    setPage(Page_Intro, new IntroPage);
    setPage(Page_AdvancedSearch, new AdvancedSearchPage);
    setPage(Page_Search, new SearchPage);
    setPage(Page_Conclusion, new ConclusionPage);

    setPixmap(QWizard::LogoPixmap, QPixmap(":/images/logo"));
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
