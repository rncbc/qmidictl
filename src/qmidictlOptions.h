// qmidictlOptions.h
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

#ifndef __qmidictlOptions_h
#define __qmidictlOptions_h

#include <QSettings>
#include <QStringList>

// Some hard-coded default options....
#define QMIDICTL_MMC_DEVICE 127
#define QMIDICTL_UDP_IPV4_ADDR "225.0.0.37"
#define QMIDICTL_UDP_IPV6_ADDR "ff12::37"
#define QMIDICTL_UDP_PORT  21928


//-------------------------------------------------------------------------
// qmidictlOptions - Prototype settings class (singleton).
//

class qmidictlOptions
{
public:

	// Constructor.
	qmidictlOptions();
	// Default destructor.
	~qmidictlOptions();

	// The settings object accessor.
	QSettings& settings();

	// Explicit I/O methods.
	void loadOptions();
	void saveOptions();

	// Command line arguments parser.
	bool parse_args(const QStringList& args);
	// Command line usage helper.
	void print_usage(const QString& arg0);

	// Network options...
	QString sInterface;
	QString sUdpAddr;
	int     iUdpPort;

	// MIDI options...
	int     iMmcDevice;

	// Singleton instance accessor.
	static qmidictlOptions *getInstance();

private:

	// Settings member variables.
	QSettings m_settings;

	// The singleton instance.
	static qmidictlOptions *g_pOptions;
};


#endif  // __qmidictlOptions_h


// end of qmidictlOptions.h
