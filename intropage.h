#ifndef INTROPAGE_H
#define INTROPAGE_H

#include <QWizardPage>

class IntroPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit IntroPage(QWidget *parent = 0);

signals:

public slots:
    void linkActivated(const QString &link);
};

#endif // INTROPAGE_H
