// qmidictlUdpDevice.cpp
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

#include "qmidictlUdpDevice.h"

#if defined(CONFIG_IPV6)

#include <QNetworkInterface>
#include <QByteArray>

#else

#include <stdlib.h>
#include <string.h>

#if defined(Q_OS_SYMBIAN)
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
inline void closesocket(int s) { ::close(s); }
#else
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
static WSADATA g_wsaData;
typedef int socklen_t;
#else
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
inline void closesocket(int s) { ::close(s); }
#endif
#endif

#include <QByteArray>
#include <QThread>


//----------------------------------------------------------------------------
// qmidictlUdpDeviceThread -- Network listener thread.
//

class qmidictlUdpDeviceThread : public QThread
{
public:

	// Constructor.
	qmidictlUdpDeviceThread(int sockin);

	// Run-state accessors.
	void setRunState(bool bRunState);
	bool runState() const;

protected:

	// The main thread executive.
	void run();

private:

	// The listener socket.
	int m_sockin;

	// Whether the thread is logically running.
	volatile bool m_bRunState;
};


// Constructor.
qmidictlUdpDeviceThread::qmidictlUdpDeviceThread ( int sockin )
	: QThread(), m_sockin(sockin), m_bRunState(false)
{
}


// Run-state accessors.
void qmidictlUdpDeviceThread::setRunState ( bool bRunState )
{
	m_bRunState = bRunState;
}

bool qmidictlUdpDeviceThread::runState (void) const
{
	return m_bRunState;
}


// The main thread executive.
void qmidictlUdpDeviceThread::run (void)
{
	m_bRunState = true;

	while (m_bRunState) {

		// Wait for an network event...
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(m_sockin, &fds);

		int fdmax = m_sockin;

		// Set timeout period (1 second)...
		struct timeval tv;
		tv.tv_sec  = 1;
		tv.tv_usec = 0;

		int s = ::select(fdmax + 1, &fds, nullptr, nullptr, &tv);
		if (s < 0) {
			::perror("select");
			break;
		}
		if (s == 0)	{
			// Timeout!
			continue;
		}

		// A Network event
		if (FD_ISSET(m_sockin, &fds)) {
			// Read from network...
			unsigned char buf[1024];
			struct sockaddr_in sender;
			socklen_t slen = sizeof(sender);
			int r = ::recvfrom(m_sockin, (char *) buf, sizeof(buf),
				0, (struct sockaddr *) &sender, &slen);
			if (r > 0)
				qmidictlUdpDevice::getInstance()->recvData(buf, r);
			else
			if (r < 0)
				::perror("recvfrom");
		}
	}
}

#endif	// !CONFIG_IPV6

 
//----------------------------------------------------------------------------
// qmidictlUdpDevice -- Network interface device (UDP/IP).
//

qmidictlUdpDevice *qmidictlUdpDevice::g_pDevice = nullptr;

// Constructor.
qmidictlUdpDevice::qmidictlUdpDevice ( QObject *pParent )
	: QObject(pParent),
#if defined(CONFIG_IPV6)
	m_sockin(nullptr), m_sockout(nullptr), m_udpport(-1)
#else
	m_sockin(-1), m_sockout(-1), m_pRecvThread(nullptr)
#endif	// !CONFIG_IPV6
{
#if !defined(CONFIG_IPV6)
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
	WSAStartup(MAKEWORD(1, 1), &g_wsaData);
#endif
#endif	// !CONFIG_IPV6

	g_pDevice = this;
}

// Destructor.
qmidictlUdpDevice::~qmidictlUdpDevice (void)
{
	close();

	g_pDevice = nullptr;

#if !defined(CONFIG_IPV6)
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
	WSACleanup();
#endif
#endif	// !CONFIG_IPV6
}


// Kind of singleton reference.
qmidictlUdpDevice *qmidictlUdpDevice::getInstance (void)
{
	return g_pDevice;
}


// Device initialization method.
bool qmidictlUdpDevice::open (
	const QString& sInterface, const QString& sUdpAddr, int iUdpPort )
{
	// Close if already open.
	close();

#if defined(CONFIG_IPV6)

	// Setup host address for udp multicast...
	m_udpaddr.setAddress(sUdpAddr);

	// Check whether is real for udp multicast...
	if (!m_udpaddr.isMulticast()) {
		qWarning() << "open(hostaddr):" << sUdpAddr
			<< "not an udp multicast address";
		return false;
	}

	// Check whether protocol is IPv4 or IPv6...
	const bool ipv6_protocol
		= (m_udpaddr.protocol() != QAbstractSocket::IPv4Protocol);

	// Setup network interface...
	QNetworkInterface iface;
	if (!sInterface.isEmpty())
		iface = QNetworkInterface::interfaceFromName(sInterface);

	// Allocate sockets and addresses...
	m_sockin  = new QUdpSocket();
	m_sockout = new QUdpSocket();

	m_udpport = iUdpPort;

	// Bind input socket...
	if (!m_sockin->bind(ipv6_protocol
			? QHostAddress::AnyIPv6
			: QHostAddress::AnyIPv4,
			iUdpPort, QUdpSocket::ShareAddress)) {
		qWarning() << "open(sockin)"
			<< "udp socket error "
			<< m_sockin->error()
			<< m_sockin->errorString();
		return false;
	}

#if defined(Q_OS_WIN)
	m_sockin->setSocketOption(
		QAbstractSocket::MulticastLoopbackOption, 0);
#endif

	if (iface.isValid())
		m_sockin->joinMulticastGroup(m_udpaddr, iface);
	else
		m_sockin->joinMulticastGroup(m_udpaddr);

	QObject::connect(m_sockin,
		SIGNAL(readyRead()),
		SLOT(readPendingDatagrams()));

	// Bind output socket...
	if (!m_sockout->bind(ipv6_protocol
			? QHostAddress::AnyIPv6
			: QHostAddress::AnyIPv4,
			m_sockout->localPort())) {
		qWarning() << "open(sockout):"
			<< "udp socket error"
			<< m_sockout->error()
			<< m_sockout->errorString();
		return false;
	}

	m_sockout->setSocketOption(
		QAbstractSocket::MulticastTtlOption, 1);
#if defined(Q_OS_UNIX)
	m_sockout->setSocketOption(
		QAbstractSocket::MulticastLoopbackOption, 0);
#endif

	if (iface.isValid())
		m_sockout->setMulticastInterface(iface);

#else

	// Setup network protocol...
	int protonum = 0;
#if 0
	struct protoent *proto = ::getprotobyname("IP");
	if (proto)
		protonum = proto->p_proto;
#endif

	// Stable interface name...
	const char *ifname = nullptr;
	const QByteArray aInterface = sInterface.toLocal8Bit();
	if (!aInterface.isEmpty())
		ifname = aInterface.constData();

	const char *udp_addr = nullptr;
	const QByteArray aUdpAddr = sUdpAddr.toLocal8Bit();
	if (!aUdpAddr.isEmpty())
		udp_addr = aUdpAddr.constData();

	// Input socket stuff...
	//
	m_sockin = ::socket(PF_INET, SOCK_DGRAM, protonum);
	if (m_sockin < 0) {
		::perror("socket(in)");
		return false;
	}

	struct sockaddr_in addrin;
	::memset(&addrin, 0, sizeof(addrin));
	addrin.sin_family = AF_INET;
	addrin.sin_addr.s_addr = htonl(INADDR_ANY);
	addrin.sin_port = htons(iUdpPort);

	if (::bind(m_sockin, (struct sockaddr *) (&addrin), sizeof(addrin)) < 0) {
		::perror("bind");
		return false;
	}

	// Will Hall, 2007
	// INADDR_ANY will bind to default interface,
	// specify alternate interface nameon which to bind...
	struct in_addr if_addr_in;
	if (ifname) {
		if (!get_address(m_sockin, &if_addr_in, ifname)) {
			fprintf(stderr, "socket(in): could not find interface address for %s\n", ifname);
			return false;
		}
		if (::setsockopt(m_sockin, IPPROTO_IP, IP_MULTICAST_IF,
				(char *) &if_addr_in, sizeof(if_addr_in))) {
			::perror("setsockopt(IP_MULTICAST_IF)");
			return false;
		}
	} else {
		if_addr_in.s_addr = htonl(INADDR_ANY);
	}

	struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = ::inet_addr(udp_addr);
	mreq.imr_interface.s_addr = if_addr_in.s_addr;
	if(::setsockopt (m_sockin, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			(char *) &mreq, sizeof(mreq)) < 0) {
		::perror("setsockopt(IP_ADD_MEMBERSHIP)");
		fprintf(stderr, "socket(in): your kernel is probably missing multicast support.\n");
		return false;
	}

	// Output socket...
	//
	m_sockout = ::socket(AF_INET, SOCK_DGRAM, protonum);
	if (m_sockout < 0) {
		::perror("socket(out)");
		return false;
	}

	// Will Hall, Oct 2007
	if (!sInterface.isEmpty()) {
		const QByteArray aInterface = sInterface.toLocal8Bit();
		const char *ifname = aInterface.constData();
		struct in_addr if_addr_out;
		if (!get_address(m_sockout, &if_addr_out, ifname)) {
			::fprintf(stderr, "socket(out): could not find interface address for %s\n", ifname);
			return false;
		}
		if (::setsockopt(m_sockout, IPPROTO_IP, IP_MULTICAST_IF,
				(char *) &if_addr_out, sizeof(if_addr_out))) {
			::perror("setsockopt(IP_MULTICAST_IF)");
			return false;
		}
	}

	::memset(&m_addrout, 0, sizeof(m_addrout));
	m_addrout.sin_family = AF_INET;
	m_addrout.sin_addr.s_addr = ::inet_addr(udp_addr);
	m_addrout.sin_port = htons(iUdpPort);

	// Turn off loopback...
	int loop = 0;
	if (::setsockopt(m_sockout, IPPROTO_IP, IP_MULTICAST_LOOP,
			(char *) &loop, sizeof (loop)) < 0) {
		::perror("setsockopt(IP_MULTICAST_LOOP)");
		return false;
	}

	// Start listener thread...
	m_pRecvThread = new qmidictlUdpDeviceThread(m_sockin);
	m_pRecvThread->start();

#endif	// !CONFIG_IPV6

	// Done.
	return true;
}


// Device termination method.
void qmidictlUdpDevice::close (void)
{
#if defined(CONFIG_IPV6)

	if (m_sockin) {
		delete m_sockin;
		m_sockin = nullptr;
	}

	if (m_sockout) {
		delete m_sockout;
		m_sockout = nullptr;
	}

	m_udpaddr.clear();
	m_udpport = -1;

#else

	if (m_sockin >= 0)
		::closesocket(m_sockin);
	if (m_sockout >= 0)
		::closesocket(m_sockout);

	if (m_pRecvThread) {
		if (m_pRecvThread->isRunning()) {
			m_pRecvThread->setRunState(false);
		//	m_pRecvThread->terminate();
			m_pRecvThread->wait(1200); // Timeout>1sec.
		}
		delete m_pRecvThread;
		m_pRecvThread = nullptr;
	}

#endif	// !CONFIG_IPV6
}


// Data transmission methods.
bool qmidictlUdpDevice::sendData ( unsigned char *data, unsigned short len ) const
{
#if defined(CONFIG_IPV6)

	if (m_sockout == nullptr)
		return false;

	if (!m_sockout->isValid()
		|| m_sockout->state() != QAbstractSocket::BoundState) {
		qWarning() << "sendData:"
			<< "udp socket has invalid state"
			<< m_sockout->state();
		return false;
	}

	QByteArray datagram((const char *) data, len);
	if (m_sockout->writeDatagram(datagram, m_udpaddr, m_udpport) < len) {
		qWarning() << "sendData:"
			<< "udp socket error"
			<< m_sockout->error() << " "
			<< m_sockout->errorString();
		return false;
	}

#else

	if (m_sockout < 0)
		return false;

	if (::sendto(m_sockout, (char *) data, len, 0,
			(struct sockaddr*) &m_addrout, sizeof(m_addrout)) < 0) {
		::perror("sendto");
		return false;
	}

#endif	// !CONFIG_IPV6

	return true;
}


#if defined(CONFIG_IPV6)

// Process incoming datagrams.
void qmidictlUdpDevice::readPendingDatagrams (void)
{
	if (m_sockin == nullptr)
		return;

	while (m_sockin->hasPendingDatagrams()) {
		QByteArray datagram;
		int nread = m_sockin->pendingDatagramSize();
		datagram.resize(nread);
		nread = m_sockin->readDatagram(datagram.data(), datagram.size());
		if (nread > 0) {
			datagram.resize(nread);
			emit received(datagram);
		}
	}
}

#else

void qmidictlUdpDevice::recvData ( unsigned char *data, unsigned short len )
{
	emit received(QByteArray((const char *) data, len));
}


// Get interface address from supplied name.
bool qmidictlUdpDevice::get_address (
	int sock, struct in_addr *inaddr, const char *ifname )
{
#if !defined(__WIN32__) && !defined(_WIN32) && !defined(WIN32) && !defined(Q_OS_SYMBIAN)

	struct ifreq ifr;
	::strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);

	if (::ioctl(sock, SIOCGIFFLAGS, (char *) &ifr)) {
		::perror("ioctl(SIOCGIFFLAGS)");
		return false;
	}

	if ((ifr.ifr_flags & IFF_UP) == 0) {
		fprintf(stderr, "interface %s is down\n", ifname);
		return false;
	}

	if (::ioctl(sock, SIOCGIFADDR, (char *) &ifr)) {
		::perror("ioctl(SIOCGIFADDR)");
		return false;
	}

	struct sockaddr_in sa;
	::memcpy(&sa, &ifr.ifr_addr, sizeof(struct sockaddr_in));
	inaddr->s_addr = sa.sin_addr.s_addr;

	return true;

#else

	return false;

#endif	// !WIN32 && !SYMBIAN
}


#endif	// !CONFIG_IPV6


// end of qmidictlUdpDevice.h
