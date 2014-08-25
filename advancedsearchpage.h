#ifndef ADVANCEDSEARCHPAGE_H
#define ADVANCEDSEARCHPAGE_H

#include <QWizardPage>

class QCheckBox;

class AdvancedSearchPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit AdvancedSearchPage(QWidget *parent = 0);

    bool isComplete() const;
    void initializePage();
protected:
    QCheckBox *arpscan;

signals:

public slots:

};

#endif // ADVANCEDSEARCHPAGE_H
