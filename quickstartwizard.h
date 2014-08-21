#ifndef QUICKSTARTWIZARD_H
#define QUICKSTARTWIZARD_H

#include <QWizard>

class QuickStartWizard : public QWizard
{
    Q_OBJECT
public:
    explicit QuickStartWizard(QWidget *parent = 0);
    enum { Page_Intro, Page_Search, Page_AdvancedSearch };

signals:

public slots:

};

#endif // QUICKSTARTWIZARD_H
