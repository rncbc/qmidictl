// qmidictl.cpp
//
/****************************************************************************
   Copyright (C) 2010-2018, rncbc aka Rui Nuno Capela. All rights reserved.

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
#include "qmidictlMidiControl.h"
#include "qmidictlMainForm.h"

#include <QApplication>

#if defined(Q_OS_SYMBIAN)
// to lock orientation in Symbian
#include <eikenv.h>
#include <eikappui.h>
#include <aknenv.h>
#include <aknappui.h>
#endif


int main ( int argc, char *argv[] )
{
	Q_INIT_RESOURCE(qmidictl);

	QApplication app(argc, argv);
#if QT_VERSION >= 0x050100
    app.setApplicationName(QMIDICTL_TITLE);
    app.setApplicationDisplayName(
		QMIDICTL_TITLE " - " + QObject::tr(QMIDICTL_SUBTITLE));
#endif
#if defined(Q_OS_ANDROID)
	const QFont& font = app.font();
	app.setFont(QFont(font.family(), font.pointSize() - 1));
#endif
	qmidictlOptions opt;
	if (!opt.parse_args(app.arguments())) {
		app.quit();
		return 1;
	}

	qmidictlMidiControl ctl;
	ctl.load(opt.settings());

	qmidictlMainForm w;

#if defined(Q_OS_SYMBIAN)
	// Lock orientation to portrait in Symbian
	CAknAppUi* appUi = dynamic_cast<CAknAppUi*> (CEikonEnv::Static()->AppUi());
	TRAP_IGNORE(
		if(appUi) {
			appUi->SetOrientationL(CAknAppUi::EAppUiOrientationLandscape);
		}
	);
	w.showMaximized();
#else
	w.show();
#endif
        w.setup();

	return app.exec();
}

// end of qmidictl.cpp

