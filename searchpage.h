#ifndef SEARCHPAGE_H
#define SEARCHPAGE_H

#include <QWizardPage>
#include <QList>
#include <QThread>
#include <QProgressBar>
#include <QTableWidget>
#include <QProcess>
#include <QList>
#include "searchworker.h"
#include "host.h"

class QProcess;

class SearchPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit SearchPage(QWidget *parent = 0);

    void initializePage();
    void cleanupPage();
    bool isComplete() const;
private:
    void addWorker(SearchWorker *worker);
    QThread searchThread;
    QProgressBar progressBar;
    QTableWidget hostsTable;
    QList<QObject *> searchWorkers;

protected:
    void resizeEvent(QResizeEvent *);

signals:
    void cleaningUp();
public slots:
    void gotHost(Host *host);
    void gotError(const QString &title, const QString &message);
    void searchFinished();
};

#endif // SEARCHPAGE_H
