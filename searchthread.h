#ifndef SEARCHTHREAD_H
#define SEARCHTHREAD_H

#include <QThread>
#include <QList>

class QProcess;

class SearchThread : public QThread
{
    Q_OBJECT
public:
    explicit SearchThread(QObject *parent = 0);
    ~SearchThread();

protected:
    bool stopping;
    QList<QProcess*> pingProcesses;

    virtual void run();
public slots:
    void stop();
};

#endif // SEARCHTHREAD_H
