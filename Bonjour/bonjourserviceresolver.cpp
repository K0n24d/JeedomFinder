/*
Copyright (c) 2007, Trenton Schulz

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 3. The name of the author may not be used to endorse or promote products
    derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <QtCore/QSocketNotifier>
#include <QtNetwork/QHostInfo>
#include <QtDebug>

#include "bonjourrecord.h"
#include "bonjourserviceresolver.h"

BonjourServiceResolver::BonjourServiceResolver(QObject *parent)
    : QObject(parent), dnssref(0), bonjourSocket(0), bonjourPort(-1), libdns_sd("dns_sd")
{
    libdns_sd.load();
}

BonjourServiceResolver::~BonjourServiceResolver()
{
    cleanupResolve();
}

void BonjourServiceResolver::cleanupResolve()
{
    if (dnssref) {
        DNSServiceRefDeallocate pDNSServiceRefDeallocate = (DNSServiceRefDeallocate) libdns_sd.resolve("DNSServiceRefDeallocate");
        if(pDNSServiceRefDeallocate)
            pDNSServiceRefDeallocate(dnssref);
        else
            qWarning() << Q_FUNC_INFO << "Could not resolve DNSServiceRefDeallocate";

        dnssref = 0;
        delete bonjourSocket;
        bonjourPort = -1;
    }
}

void BonjourServiceResolver::resolveBonjourRecord(const BonjourRecord &record)
{
    qDebug() << Q_FUNC_INFO << record.registeredType << record.replyDomain << record.serviceName;

    if (dnssref) {
        qWarning("resolve in process, aborting");
        return;
    }
    DNSServiceResolve pDNSServiceResolve = (DNSServiceResolve) libdns_sd.resolve("DNSServiceResolve");
    DNSServiceErrorType err = -1;
    if(pDNSServiceResolve)
        err = pDNSServiceResolve(&dnssref, 0, 0,
                                                record.serviceName.toUtf8().constData(),
                                                record.registeredType.toUtf8().constData(),
                                                record.replyDomain.toUtf8().constData(),
                                                (DNSServiceResolveReply)bonjourResolveReply, this);
    else
        qWarning() << Q_FUNC_INFO << "Could not resolve DNSServiceResolve";

    if (err != kDNSServiceErr_NoError) {
        qDebug() << Q_FUNC_INFO << err;
        emit error(err);
    } else {
        DNSServiceRefSockFD pDNSServiceRefSockFD = (DNSServiceRefSockFD) libdns_sd.resolve("DNSServiceRefSockFD");
        int sockfd = -1;
        if(pDNSServiceRefSockFD)
            sockfd = pDNSServiceRefSockFD(dnssref);
        else
            qWarning() << Q_FUNC_INFO << "Could not resolve DNSServiceRefSockFD";
        if (sockfd == -1) {
            qDebug() << Q_FUNC_INFO << sockfd;
            emit error(kDNSServiceErr_Invalid);
        } else {
            bonjourSocket = new QSocketNotifier(sockfd, QSocketNotifier::Read, this);
            connect(bonjourSocket, SIGNAL(activated(int)), this, SLOT(bonjourSocketReadyRead()));
        }
    }
}

void BonjourServiceResolver::bonjourSocketReadyRead()
{
    DNSServiceProcessResult pDNSServiceProcessResult = (DNSServiceProcessResult) libdns_sd.resolve("DNSServiceProcessResult");
    DNSServiceErrorType err = -1;
    if(pDNSServiceProcessResult)
        err = pDNSServiceProcessResult(dnssref);
    else
        qWarning() << Q_FUNC_INFO << "Could not resolve DNSServiceProcessResult";

    if (err != kDNSServiceErr_NoError)
        emit error(err);
}


void BonjourServiceResolver::bonjourResolveReply(DNSServiceRef, DNSServiceFlags ,
                                    quint32 , DNSServiceErrorType errorCode,
                                    const char *hostname, const char *hosttarget, quint16 port,
                                    quint16 txtLen, const unsigned char *txtRecord, void *context)
{
    qDebug() << Q_FUNC_INFO << hostname << hosttarget;
    BonjourServiceResolver *serviceResolver = static_cast<BonjourServiceResolver *>(context);
    if (errorCode != kDNSServiceErr_NoError) {
        qDebug() << Q_FUNC_INFO << errorCode;
        emit serviceResolver->error(errorCode);
        return;
    }
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        {
            port =  0 | ((port & 0x00ff) << 8) | ((port & 0xff00) >> 8);
        }
#endif
    serviceResolver->bonjourPort = port;
    serviceResolver->bonjourTXT = QString::fromUtf8((const char *)(txtRecord), txtLen);
    serviceResolver->bonjourHostName = QString::fromUtf8(hostname);
    serviceResolver->bonjourHostTarget = QString::fromUtf8(hosttarget);
    QHostInfo::lookupHost(QString::fromUtf8(hosttarget),
                          serviceResolver, SLOT(finishConnect(const QHostInfo &)));
}

void BonjourServiceResolver::finishConnect(const QHostInfo &hostInfo)
{
    emit bonjourRecordResolved(hostInfo, bonjourPort, bonjourTXT);
    QMetaObject::invokeMethod(this, "cleanupResolve", Qt::QueuedConnection);
}
