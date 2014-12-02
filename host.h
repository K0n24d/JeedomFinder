#ifndef HOST_H
#define HOST_H

#include <QObject>

class Host : public QObject
{
    Q_OBJECT
public:
    explicit Host(const Host * host, QObject *parent = 0);
    explicit Host(QObject *parent = 0);

    QString name;
    QString ip;
    QString desc;
    QString url;

signals:

public slots:

};

//Q_DECLARE_METATYPE(Host);

#endif // HOST_H
