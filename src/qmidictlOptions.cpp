// qmidictlOptions.cpp
//
/****************************************************************************
   Copyright (C) 2010-2025, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include <QApplication>

#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
#include <QCommandLineParser>
#include <QCommandLineOption>
#if defined(Q_OS_WINDOWS)
#include <QMessageBox>
#endif
#endif


//-------------------------------------------------------------------------
// qmidictlOptions - Prototype settings structure (pseudo-singleton).
//

// Singleton instance pointer.
qmidictlOptions *qmidictlOptions::g_pOptions = nullptr;

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
	g_pOptions = nullptr;
}


// Explicit load method.
void qmidictlOptions::loadOptions (void)
{
	// And go into general options group.
	m_settings.beginGroup("/Options");

	// Network specific options...
	m_settings.beginGroup("/Network");
	sInterface = m_settings.value("/Interface").toString();
	sUdpAddr = m_settings.value("/UdpAddr", QMIDICTL_UDP_IPV4_ADDR).toString();
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


#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)

void qmidictlOptions::show_error( const QString& msg )
{
#if defined(Q_OS_WINDOWS)
	QMessageBox::information(nullptr, QApplication::applicationName(), msg);
#else
	const QByteArray tmp = msg.toUtf8() + '\n';
	::fputs(tmp.constData(), stderr);
#endif
}

#else

// Help about command line options.
void qmidictlOptions::print_usage ( const QString& arg0 )
{
	QTextStream out(stderr);
	const QString sEot = "\n\t";
	const QString sEol = "\n\n";

	out << QObject::tr("Usage: %1 [options]").arg(arg0) + sEol;
	out << QMIDICTL_TITLE " - " << QObject::tr(QMIDICTL_SUBTITLE) + sEol;
	out << QObject::tr("Options:") + sEol;
#if !defined(Q_OS_SYMBIAN) && !defined(Q_OS_WINDOWS)
	out << "  -i, --interface=[interface]" + sEot +
		QObject::tr("Use specific network interface (default = %1)")
			.arg(sInterface.isEmpty() ? "all" : sInterface) + sEol;
#endif
	out << "  -u, --udp-addr=[addr]" + sEot +
		QObject::tr("Use specific network address (default = %1)")
			.arg(sUdpAddr) + sEol;
	out << "  -p, --udp-port=[port]" + sEot +
		QObject::tr("Use specific network port (default = %1)")
			.arg(iUdpPort) + sEol;
	out << "  -m, --mmc-device=[num]" + sEot +
		QObject::tr("Use specific MMC device number (default = %1)")
			.arg(iMmcDevice) + sEol;
	out << "  -h, --help" + sEot +
		QObject::tr("Show help about command line options.") + sEol;
	out << "  -v, --version" + sEot +
		QObject::tr("Show version information.") + sEol;
}

#endif


// Parse command line arguments into m_settings.
bool qmidictlOptions::parse_args ( const QStringList& args )
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)

	QCommandLineParser parser;
	parser.setApplicationDescription(
		QMIDICTL_TITLE " - " + QObject::tr(QMIDICTL_SUBTITLE));

#if !defined(Q_OS_SYMBIAN) && !defined(Q_OS_WINDOWS)
	parser.addOption({{"i", "interface"},
		QObject::tr("Use specific network interface (default = %1)")
			.arg(sInterface.isEmpty() ? "all" : sInterface), "name"});
#endif
	parser.addOption({{"u", "udp-addr"},
		QObject::tr("Use specific network address (default = %1)")
			.arg(sUdpAddr), "addr"});
	parser.addOption({{"p", "udp-port"},
		QObject::tr("Use specific network port (default = %1)")
			.arg(iUdpPort), "port"});
	parser.addOption({{"m", "mmc-device"},
		QObject::tr("Use specific MMC device number (default = %1)")
			.arg(iMmcDevice), "num"});
	const QCommandLineOption& helpOption = parser.addHelpOption();
	const QCommandLineOption& versionOption = parser.addVersionOption();

	if (!parser.parse(args)) {
		show_error(parser.errorText());
		return false;
	}

	if (parser.isSet(helpOption)) {
		show_error(parser.helpText());
		return false;
	}

	if (parser.isSet(versionOption)) {
		QString sVersion = QString("%1 %2\n")
			.arg(QMIDICTL_TITLE)
			.arg(QCoreApplication::applicationVersion());
		sVersion += QString("Qt: %1").arg(qVersion());
	#if defined(QT_STATIC)
		sVersion += "-static";
	#endif
		sVersion += '\n';
		show_error(sVersion);
		return false;
	}

#if !defined(Q_OS_SYMBIAN) && !defined(Q_OS_WINDOWS)
	if (parser.isSet("interface")) {
		sInterface = parser.value("interface"); // Maybe empty!
	}
#endif
	if (parser.isSet("udp-addr")) {
		const QString& sVal = parser.value("udp-addr");
		if (sVal.isEmpty()) {
			show_error(QObject::tr("Option -u requires an argument (addr)."));
			return false;
		}
		sUdpAddr = sVal;
	}

	if (parser.isSet("udp-port")) {
		bool bOK = false;
		const int iVal = parser.value("udp-port").toInt(&bOK);
		if (!bOK) {
			show_error(QObject::tr("Option -p requires an argument (port)."));
			return false;
		}
		iUdpPort = iVal;
	}

	if (parser.isSet("mmc-device")) {
		bool bOK = false;
		const int iVal = parser.value("mmc-device").toInt(&bOK);
		if (!bOK) {
			show_error(QObject::tr("Option -m requires an argument (num)."));
			return false;
		}
		iMmcDevice = iVal;
	}

#else

	QTextStream out(stderr);
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
		if (sArg == "-u" || sArg == sArg == "--udp-addr") {
			if (sVal.isEmpty()) {
				out << QObject::tr("Option -u requires an argument (addr).") + sEol;
				return false;
			}
			sUdpAddr = sVal;
			if (iEqual < 0) ++i;
		}
		else
		if (sArg == "-p" || sArg == "--udp-port") {
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
				out << QObject::tr("Option -m requires an argument (num).") + sEol;
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
			out << QString("%1: %2\n")
				.arg(QMIDICTL_TITLE)
				.arg(PROJECT_VERSION);
			out << QString("Qt: %1").arg(qVersion());
		#if defined(QT_STATIC)
			out << "-static";
		#endif
			out << '\n';
			return false;
		}
	}

#endif

	// Alright with argument parsing.
	return true;
}


// end of qmidictlOptions.cpp
