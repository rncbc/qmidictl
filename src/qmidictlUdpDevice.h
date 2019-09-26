// qmidictlUdpDevice.h
//
/****************************************************************************
   Copyright (C) 2010-2019, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#ifndef __qmidictlUdpDevice_h
#define __qmidictlUdpDevice_h

#include "qmidictlAbout.h"

#include <stdio.h>

#if !defined(CONFIG_IPV6)
#if defined(__SYMBIAN32__)
#include <sys/socket.h>
#include <netinet/in.h>
#else
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
#include <winsock.h>
#else
#include <arpa/inet.h>
#include <net/if.h>
#endif
#endif
#endif	// !CONFIG_IPV6

#include <QObject>
#include <QString>

#if defined(CONFIG_IPV6)
#include <QUdpSocket>
#include <QHostAddress>
#endif


//----------------------------------------------------------------------------
// qmidictlUdpDevice -- Network interface device (UDP/IP).

class qmidictlUdpDevice : public QObject
{
	Q_OBJECT

public:

	// Constructor.
	qmidictlUdpDevice(QObject *pParent = nullptr);

	// Destructor.
	~qmidictlUdpDevice();

	// Kind of singleton reference.
	static qmidictlUdpDevice *getInstance();

	// Device initialization method.
	bool open(const QString& sInterface, const QString& sUdpAddr, int iUdpPort);

	// Device termination method.
	void close();

	// Data transmission methods.
	bool sendData(unsigned char *data, unsigned short len) const;
	void recvData(unsigned char *data, unsigned short len);

signals:

	// Received data signal.
	void received(const QByteArray& data);

#if defined(CONFIG_IPV6)
protected slots:

	// Process incoming datagrams.
	void readPendingDatagrams();

#else
protected:

	// Get interface address from supplied name.
	static bool get_address(int sock, struct in_addr *iaddr, const char *ifname);

#endif	// !CONFIG_IPV6

private:

	// Instance variables,
#if defined(CONFIG_IPV6)

	QUdpSocket *m_sockin;
	QUdpSocket *m_sockout;

	QHostAddress m_udpaddr;

	int m_udpport;

#else

	int m_sockin;
	int m_sockout;

	struct sockaddr_in m_addrout;

	// Network receiver thread.
	class qmidictlUdpDeviceThread *m_pRecvThread;

#endif	// !CONFIG_IPV6

	// Kind-of singleton reference.
	static qmidictlUdpDevice *g_pDevice;
};


#endif	// __qmidictlUdpDevice_h


// end of qmidictlUdpDevice.h
