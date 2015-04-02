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

#include "bonjourservicebrowser.h"

#include <QtCore/QSocketNotifier>
#include <QtDebug>

BonjourServiceBrowser::BonjourServiceBrowser(QObject *parent)
    : QObject(parent), dnssref(0), bonjourSocket(0), libdns_sd("dns_sd")
{
    dnssref=NULL;
    bonjourSocket=NULL;
    libdns_sd.load();
}

BonjourServiceBrowser::~BonjourServiceBrowser()
{
    if (dnssref) {
        DNSServiceRefDeallocate pDNSServiceRefDeallocate = (DNSServiceRefDeallocate) libdns_sd.resolve("DNSServiceRefDeallocate");
        if(pDNSServiceRefDeallocate)
            pDNSServiceRefDeallocate(dnssref);
        else
            qWarning() << Q_FUNC_INFO << "Could not resolve DNSServiceRefDeallocate";
        dnssref = 0;
    }
}

void BonjourServiceBrowser::browseForServiceType(const QString &serviceType)
{
    DNSServiceBrowse pDNSServiceBrowse = (DNSServiceBrowse) libdns_sd.resolve("DNSServiceBrowse");
    DNSServiceErrorType err = -1;
    if(pDNSServiceBrowse)
        err = pDNSServiceBrowse(&dnssref, 0, 0, serviceType.toUtf8().constData(), NULL,
                                               bonjourBrowseReply, this);
    else
        qWarning() << Q_FUNC_INFO << "Could not resolve DNSServiceBrowse";
    if (err != kDNSServiceErr_NoError) {
        emit error(err);
    } else {
        DNSServiceRefSockFD pDNSServiceRefSockFD = (DNSServiceRefSockFD) libdns_sd.resolve("DNSServiceRefSockFD");
        int sockfd = -1;
        if(pDNSServiceRefSockFD)
            sockfd = pDNSServiceRefSockFD(dnssref);
        else
            qWarning() << Q_FUNC_INFO << "Could not resolve DNSServiceRefSockFD";
        if (sockfd == -1) {
            emit error(kDNSServiceErr_Invalid);
        } else {
            bonjourSocket = new QSocketNotifier(sockfd, QSocketNotifier::Read, this);
            connect(bonjourSocket, SIGNAL(activated(int)), this, SLOT(bonjourSocketReadyRead()));
        }
    }
}

void BonjourServiceBrowser::bonjourSocketReadyRead()
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

void BonjourServiceBrowser::bonjourBrowseReply(DNSServiceRef , DNSServiceFlags flags,
                                               quint32 , DNSServiceErrorType errorCode,
                                               const char *serviceName, const char *regType,
                                               const char *replyDomain, void *context)
{
    BonjourServiceBrowser *serviceBrowser = static_cast<BonjourServiceBrowser *>(context);
    if (errorCode != kDNSServiceErr_NoError) {
        emit serviceBrowser->error(errorCode);
    } else {
        BonjourRecord bonjourRecord(serviceName, regType, replyDomain);
        if (flags & kDNSServiceFlagsAdd) {
            if (!serviceBrowser->bonjourRecords.contains(bonjourRecord))
                serviceBrowser->bonjourRecords.append(bonjourRecord);
        } else {
            serviceBrowser->bonjourRecords.removeAll(bonjourRecord);
        }
        if (!(flags & kDNSServiceFlagsMoreComing)) {
            emit serviceBrowser->currentBonjourRecordsChanged(serviceBrowser->bonjourRecords);
        }
    }
}
