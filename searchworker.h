#ifndef SEARCHWORKER_H
#define SEARCHWORKER_H

#include <QObject>

class SearchWorker : public QObject
{
    Q_OBJECT
public:
    explicit SearchWorker(QObject *parent = 0);
    struct Host {
        QString name;
        QString ip;
        QString desc;
        QString url;
    };

signals:
    void finished();
    void error(const QString & title, const QString & message);
    void host(const SearchWorker::Host & host);

public slots:
    virtual void discover(){};
    virtual void stop(){};
};


#endif // SEARCHWORKER_H
