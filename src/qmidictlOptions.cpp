// qmidictlOptions.cpp
//
/****************************************************************************
   Copyright (C) 2010-2016, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "qmidictlAbout.h"
#include "qmidictlOptions.h"

#include <QTextStream>


//-------------------------------------------------------------------------
// qmidictlOptions - Prototype settings structure (pseudo-singleton).
//

// Singleton instance pointer.
qmidictlOptions *qmidictlOptions::g_pOptions = NULL;

// Singleton instance accessor (static).
qmidictlOptions *qmidictlOptions::getInstance (void)
{
	return g_pOptions;
}


// Constructor.
qmidictlOptions::qmidictlOptions (void)
	: m_settings(QMIDICTL_DOMAIN, QMIDICTL_TITLE)
{
	// Pseudo-singleton reference setup.
	g_pOptions = this;

	loadOptions();
}


// Default Destructor.
qmidictlOptions::~qmidictlOptions (void)
{
	saveOptions();

	// Pseudo-singleton reference shut-down.
	g_pOptions = NULL;
}


// Explicit load method.
void qmidictlOptions::loadOptions (void)
{
	// And go into general options group.
	m_settings.beginGroup("/Options");

	// Network specific options...
	m_settings.beginGroup("/Network");
	sInterface = m_settings.value("/Interface").toString();
	sUdpAddr = m_settings.value("/UdpAddr", QMIDICTL_UDP_ADDR).toString();
	iUdpPort = m_settings.value("/UdpPort", QMIDICTL_UDP_PORT).toInt();
	m_settings.endGroup();

	// MIDI specific options...
	m_settings.beginGroup("/Midi");
	iMmcDevice = m_settings.value("/MmcDevice", 0x7f).toInt();
	m_settings.endGroup();

	m_settings.endGroup();
}


// Explicit save method.
void qmidictlOptions::saveOptions (void)
{
	// And go into general options group.
	m_settings.beginGroup("/Options");

	// Network specific options...
	m_settings.beginGroup("/Network");
	m_settings.setValue("/Interface", sInterface);
	m_settings.setValue("/UdpAddr", sUdpAddr);
	m_settings.setValue("/UdpPort", iUdpPort);
	m_settings.endGroup();

	// MIDI specific options...
	m_settings.beginGroup("/Midi");
	m_settings.setValue("/MmcDevice", iMmcDevice);
	m_settings.endGroup();

	m_settings.endGroup();

	// Save/commit to disk.
	m_settings.sync();
}


// Settings accessor.
QSettings& qmidictlOptions::settings (void)
{
	return m_settings;
}


// Help about command line options.
void qmidictlOptions::print_usage ( const QString& arg0 )
{
	QTextStream out(stderr);
	const QString sEot = "\n\t";
	const QString sEol = "\n\n";

	out << QMIDICTL_TITLE " - " << QObject::tr(QMIDICTL_SUBTITLE) + sEol;
	out << QObject::tr("Usage: %1 [options]").arg(arg0) + sEol;
	out << QObject::tr("Options:") + sEol;
#if !defined(Q_OS_SYMBIAN) && !defined(Q_OS_WINDOWS)
    out << "  -i, --interface=[interface]" + sEot +
		QObject::tr("Use specific network interface (default = %1)")
			.arg(sInterface.isEmpty() ? "all" : sInterface) + sEol;
#endif
	out << "  -u, --address, --udp-addr=[address]" + sEot +
		QObject::tr("Use specific network address (default = %1)")
			.arg(sUdpAddr) + sEol;
	out << "  -p, --port, --udp-port=[port]" + sEot +
		QObject::tr("Use specific network port (default = %1)")
			.arg(iUdpPort) + sEol;
	out << "  -m, --mmc-device=[mmc-device]" + sEot +
		QObject::tr("Use specific MMC device number (default = %1)")
			.arg(iMmcDevice) + sEol;
	out << "  -h, --help" + sEot +
		QObject::tr("Show help about command line options") + sEol;
	out << "  -v, --version" + sEot +
		QObject::tr("Show version information") + sEol;
}


// Parse command line arguments into m_settings.
bool qmidictlOptions::parse_args ( const QStringList& args )
{
	QTextStream out(stderr);
	const QString sEot = "\n\t";
	const QString sEol = "\n\n";
	const int argc = args.count();

	for (int i = 1; i < argc; ++i) {

		QString sVal;
		QString sArg = args.at(i);
		const int iEqual = sArg.indexOf('=');
		if (iEqual >= 0) {
			sVal = sArg.right(sArg.length() - iEqual - 1);
			sArg = sArg.left(iEqual);
		}
		else if (i < argc - 1) {
			sVal = args.at(i + 1);
			if (sVal[0] == '-')
				sVal.clear();
		}

	#if !defined(Q_OS_SYMBIAN) && !defined(Q_OS_WINDOWS)
		if (sArg == "-i" || sArg == "--interface") {
			sInterface = sVal; // Maybe empty!
			if (iEqual < 0)
				i++;
		}
		else
	#endif
		if (sArg == "-u" || sArg == "--address" || sArg == "--udp-addr") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -d requires an argument (address).") + sEol;
				return false;
			}
			sUdpAddr = sVal;
			if (iEqual < 0) ++i;
		}
		else
		if (sArg == "-p" || sArg == "--port" || sArg == "--udp-port") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -p requires an argument (port).") + sEol;
				return false;
			}
			iUdpPort = sVal.toInt();
			if (iEqual < 0) ++i;
		}
		else
		if (sArg == "-m" || sArg == "--mmc-device") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -m requires an argument (mmc-device).") + sEol;
				return false;
			}
			iMmcDevice = sVal.toInt();
			if (iEqual < 0)
				i++;
		}
		else
		if (sArg == "-h" || sArg == "--help") {
			print_usage(args.at(0));
			return false;
		}
		else if (sArg == "-v" || sArg == "--version") {
			out << QObject::tr("Qt: %1\n")
				.arg(qVersion());
			out << QObject::tr("%1: %2  (%3)\n")
				.arg(QMIDICTL_TITLE)
				.arg(CONFIG_BUILD_VERSION)
				.arg(CONFIG_BUILD_DATE);
			return false;
		}
	}

	// Alright with argument parsing.
	return true;
}


// end of qmidictlOptions.cpp
