#include "pingsearchworker.h"
#include <QProcess>
#include <QNetworkInterface>
#include <QCoreApplication>

PingSearchWorker::PingSearchWorker(QObject *parent) :
    QObject(parent), stopping(false)
{
}

PingSearchWorker::~PingSearchWorker()
{
    foreach(QProcess *process, pingProcesses)
    {
        if(process->state()!=QProcess::NotRunning)
            process->kill();
        process->deleteLater();
    }
}

void PingSearchWorker::discover()
{
    /*
#if defined(Q_OS_UNIX)
    QString ping("/usr/bin/ping");
#elif defined(Q_OS_WIN)
    QString ping("ping.exe");
#endif
    */
    QStringList arguments;
//    arguments << "-c1" << "-n" <<"-q";

    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
    {
        if(   !interface.flags().testFlag(QNetworkInterface::IsUp)
           || interface.flags().testFlag(QNetworkInterface::IsLoopBack)
           || interface.flags().testFlag(QNetworkInterface::IsPointToPoint)
              )
            continue;

        foreach(QNetworkAddressEntry addressEntry, interface.addressEntries())
        {
            if(addressEntry.ip().protocol()!=QAbstractSocket::IPv4Protocol)
                continue;

            quint32 myaddress = addressEntry.ip().toIPv4Address();
            quint32 netmask = addressEntry.netmask().toIPv4Address();
            quint32 network = myaddress & netmask;
            quint32 address = network + 1;

            while((address & netmask) == network)
            {
                QHostAddress hostaddress(address);

                while(pingProcesses.count()>50)
                {
                    foreach(QProcess *process, pingProcesses)
                    {
                        if(stopping)
                            return;

                        if(process->state()==QProcess::NotRunning)
                        {
                            process->deleteLater();
                            pingProcesses.removeOne(process);
                        }
                    }

                    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                }

                QProcess *process = new QProcess(this);
                pingProcesses << process;
                arguments.clear();
#if defined(Q_OS_UNIX)
                arguments << "-c1" << "-n" <<"-q" << hostaddress.toString();
#elif defined(Q_OS_WIN)
                arguments << "-n" << "1" << hostaddress.toString();
#endif
                process->start("ping", arguments);
                if(!process->waitForStarted())
                {
                    emit(error(Q_FUNC_INFO, tr("Impossible de lancer ping %1").arg(process->errorString())));
                }

                address++;
            }
        }
    }

    while(pingProcesses.count()>0)
    {
        foreach(QProcess *process, pingProcesses)
        {
            if(stopping)
                return;

            if(process->state()==QProcess::NotRunning)
            {
                process->deleteLater();
                pingProcesses.removeOne(process);
            }
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    emit(finished());
}

void PingSearchWorker::stop()
{
    stopping = true;
}
