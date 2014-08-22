#ifndef SEARCHPAGE_H
#define SEARCHPAGE_H

#include <QWizardPage>
#include <QTimer>
#include <QList>
#include <QThread>
#include <QProgressBar>
#include <QProcess>

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
    QTimer checkResultsTimer;
    QThread searchThread;
    QProgressBar progressBar;
#ifdef Q_OS_WIN
    QProcess arpTableProcess;
#endif
    int numberOfSearchWorkersRunning;

signals:
    void cleaningUp();
    void hostFound(QString name, QString mac);
public slots:
    void checkResults();
#ifdef Q_OS_WIN
    void gotArpResults(int);
#endif
    void searchFinished();
};

#endif // SEARCHPAGE_H
