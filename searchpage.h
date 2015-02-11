#ifndef SEARCHPAGE_H
#define SEARCHPAGE_H

#include <QWizardPage>
#include <QList>
#include <QThread>
#include <QProgressBar>
#include <QTableWidget>
#include <QProcess>
#include <QList>
#include <QMutex>
#include <QHash>
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
    QList<QThread*> searchThreads;
    QProgressBar progressBar;
    QTableWidget hostsTable;
    QList<SearchWorker*> searchWorkers;
    QMutex tableMutex;
    QMutex searchWorkersMutex;

protected:
    void resizeEvent(QResizeEvent *);
    QHash<QString, Host*> hosts;
    bool isCleaningUp;

signals:
    void cleaningUp();
public slots:
    void gotHost(Host *newHost);
    void gotError(const QString &title, const QString &message);
    void searchFinished();
};

#endif // SEARCHPAGE_H
