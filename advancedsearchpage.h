#ifndef ADVANCEDSEARCHPAGE_H
#define ADVANCEDSEARCHPAGE_H

#include <QWizardPage>
#include <QList>

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
    QCheckBox *udp;
    QList<QCheckBox *> ipCheckBoxes;

signals:

public slots:
    void ipAddressesChanged();
    void udpChanged();
};

#endif // ADVANCEDSEARCHPAGE_H
