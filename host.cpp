#include "host.h"

Host::Host(const Host *host, QObject *parent) :
    QObject(parent), name(host->name), ip(host->ip), desc(host->desc), url(host->url)
{

}

Host::Host(QObject *parent) :
    QObject(parent)
{
}
