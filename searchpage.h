#ifndef SEARCHPAGE_H
#define SEARCHPAGE_H

#include <QWizardPage>
#include <QTimer>
#include <QList>

class SearchThread;

class SearchPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit SearchPage(QWidget *parent = 0);

    void initializePage();
    void cleanupPage();
    bool isComplete() const;
private:
    QTimer checkResultsTimer;
    SearchThread *searchThread;
signals:

public slots:
    void checkResults();
};

#endif // SEARCHPAGE_H
