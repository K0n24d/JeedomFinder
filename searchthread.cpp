#include "searchthread.h"
#include <QProcess>
#include <QNetworkInterface>
#include <QMessageBox>
#include <QCoreApplication>

SearchThread::SearchThread(QObject *parent) :
    QThread(parent), stopping(false)
{
}

SearchThread::~SearchThread()
{
    foreach(QProcess *process, pingProcesses)
    {
        if(process->state()!=QProcess::NotRunning)
            process->kill();
        delete process;
    }
}

void SearchThread::run()
{
    QStringList arguments;

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
                            delete process;
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
                    QMessageBox::warning(NULL, tr("Impossible de lancer ping"), "Erreur: " + process->errorString(), QMessageBox::Close);
                }

                address++;
            }
        }
    }
}

void SearchThread::stop()
{
    stopping=true;
}
