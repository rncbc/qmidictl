// qmidictl.cpp
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

#include "qmidictlAbout.h"

#include "qmidictlOptions.h"
#include "qmidictlMidiControl.h"
#include "qmidictlMainForm.h"

#include <QApplication>

#if defined(Q_OS_ANDROID)
#include "qmidictlActionBarStyle.h"
#include <QStyleFactory>
#endif

#if defined(Q_OS_SYMBIAN)
// to lock orientation in Symbian
#include <eikenv.h>
#include <eikappui.h>
#include <aknenv.h>
#include <aknappui.h>
#endif

#if defined(Q_OS_ANDROID)

//-------------------------------------------------------------------------
// update_palette() - Application palette settler.
//

static void update_palette ( QPalette& pal )
{
	pal.setColor(QPalette::Active,   QPalette::Window, QColor(73, 78, 88));
	pal.setColor(QPalette::Inactive, QPalette::Window, QColor(73, 78, 88));
	pal.setColor(QPalette::Disabled, QPalette::Window, QColor(64, 68, 77));
	pal.setColor(QPalette::Active,   QPalette::WindowText, QColor(182, 193, 208));
	pal.setColor(QPalette::Inactive, QPalette::WindowText, QColor(182, 193, 208));
	pal.setColor(QPalette::Disabled, QPalette::WindowText, QColor(97, 104, 114));
	pal.setColor(QPalette::Active,   QPalette::Base, QColor(60, 64, 72));
	pal.setColor(QPalette::Inactive, QPalette::Base, QColor(60, 64, 72));
	pal.setColor(QPalette::Disabled, QPalette::Base, QColor(52, 56, 63));
	pal.setColor(QPalette::Active,   QPalette::AlternateBase, QColor(67, 71, 80));
	pal.setColor(QPalette::Inactive, QPalette::AlternateBase, QColor(67, 71, 80));
	pal.setColor(QPalette::Disabled, QPalette::AlternateBase, QColor(59, 62, 70));
	pal.setColor(QPalette::Active,   QPalette::ToolTipBase, QColor(182, 193, 208));
	pal.setColor(QPalette::Inactive, QPalette::ToolTipBase, QColor(182, 193, 208));
	pal.setColor(QPalette::Disabled, QPalette::ToolTipBase, QColor(182, 193, 208));
	pal.setColor(QPalette::Active,   QPalette::ToolTipText, QColor(42, 44, 48));
	pal.setColor(QPalette::Inactive, QPalette::ToolTipText, QColor(42, 44, 48));
	pal.setColor(QPalette::Disabled, QPalette::ToolTipText, QColor(42, 44, 48));
	pal.setColor(QPalette::Active,   QPalette::Text, QColor(210, 222, 240));
	pal.setColor(QPalette::Inactive, QPalette::Text, QColor(210, 222, 240));
	pal.setColor(QPalette::Disabled, QPalette::Text, QColor(99, 105, 115));
	pal.setColor(QPalette::Active,   QPalette::Button, QColor(82, 88, 99));
	pal.setColor(QPalette::Inactive, QPalette::Button, QColor(82, 88, 99));
	pal.setColor(QPalette::Disabled, QPalette::Button, QColor(72, 77, 87));
	pal.setColor(QPalette::Active,   QPalette::ButtonText, QColor(210, 222, 240));
	pal.setColor(QPalette::Inactive, QPalette::ButtonText, QColor(210, 222, 240));
	pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(111, 118, 130));
	pal.setColor(QPalette::Active,   QPalette::BrightText, QColor(255, 255, 255));
	pal.setColor(QPalette::Inactive, QPalette::BrightText, QColor(255, 255, 255));
	pal.setColor(QPalette::Disabled, QPalette::BrightText, QColor(255, 255, 255));
	pal.setColor(QPalette::Active,   QPalette::Light, QColor(95, 101, 114));
	pal.setColor(QPalette::Inactive, QPalette::Light, QColor(95, 101, 114));
	pal.setColor(QPalette::Disabled, QPalette::Light, QColor(86, 92, 104));
	pal.setColor(QPalette::Active,   QPalette::Midlight, QColor(84, 90, 101));
	pal.setColor(QPalette::Inactive, QPalette::Midlight, QColor(84, 90, 101));
	pal.setColor(QPalette::Disabled, QPalette::Midlight, QColor(75, 81, 91));
	pal.setColor(QPalette::Active,   QPalette::Dark, QColor(40, 43, 49));
	pal.setColor(QPalette::Inactive, QPalette::Dark, QColor(40, 43, 49));
	pal.setColor(QPalette::Disabled, QPalette::Dark, QColor(35, 38, 43));
	pal.setColor(QPalette::Active,   QPalette::Mid, QColor(63, 68, 76));
	pal.setColor(QPalette::Inactive, QPalette::Mid, QColor(63, 68, 76));
	pal.setColor(QPalette::Disabled, QPalette::Mid, QColor(56, 59, 67));
	pal.setColor(QPalette::Active,   QPalette::Shadow, QColor(29, 31, 35));
	pal.setColor(QPalette::Inactive, QPalette::Shadow, QColor(29, 31, 35));
	pal.setColor(QPalette::Disabled, QPalette::Shadow, QColor(25, 27, 30));
	pal.setColor(QPalette::Active,   QPalette::Highlight, QColor(120, 136, 156));
	pal.setColor(QPalette::Inactive, QPalette::Highlight, QColor(81, 90, 103));
	pal.setColor(QPalette::Disabled, QPalette::Highlight, QColor(64, 68, 77));
	pal.setColor(QPalette::Active,   QPalette::HighlightedText, QColor(209, 225, 244));
	pal.setColor(QPalette::Inactive, QPalette::HighlightedText, QColor(182, 193, 208));
	pal.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(97, 104, 114));
	pal.setColor(QPalette::Active,   QPalette::Link, QColor(156, 212, 255));
	pal.setColor(QPalette::Inactive, QPalette::Link, QColor(156, 212, 255));
	pal.setColor(QPalette::Disabled, QPalette::Link, QColor(82, 102, 119));
	pal.setColor(QPalette::Active,   QPalette::LinkVisited, QColor(64, 128, 255));
	pal.setColor(QPalette::Inactive, QPalette::LinkVisited, QColor(64, 128, 255));
	pal.setColor(QPalette::Disabled, QPalette::LinkVisited, QColor(54, 76, 119));

	// Dark themes grayed/disabled color group fix...
	if (pal.base().color().value() < 0x7f) {
		const QColor& color = pal.window().color();
		const int iGroups = int(QPalette::Active | QPalette::Inactive) + 1;
		for (int i = 0; i < iGroups; ++i) {
			const QPalette::ColorGroup group = QPalette::ColorGroup(i);
			pal.setBrush(group, QPalette::Light,    color.lighter(140));
			pal.setBrush(group, QPalette::Midlight, color.lighter(100));
			pal.setBrush(group, QPalette::Mid,      color.lighter(90));
			pal.setBrush(group, QPalette::Dark,     color.darker(160));
			pal.setBrush(group, QPalette::Shadow,   color.darker(180));
		}
		pal.setColorGroup(QPalette::Disabled,
			pal.windowText().color().darker(),
			pal.button(),
			pal.light(),
			pal.dark(),
			pal.mid(),
			pal.text().color().darker(),
			pal.text().color().lighter(),
			pal.base(),
			pal.window());
	#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
		pal.setColor(QPalette::Disabled,
			QPalette::Highlight, pal.mid().color());
		pal.setColor(QPalette::Disabled,
			QPalette::ButtonText, pal.mid().color());
	#endif
	}
}

#endif	// Q_OS_ANDROID


//-------------------------------------------------------------------------
// main - The main program trunk.
//

int main ( int argc, char *argv[] )
{
	Q_INIT_RESOURCE(qmidictl);

#if !defined(Q_OS_ANDROID)
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#endif

	QApplication app(argc, argv);
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
	app.setApplicationName(QMIDICTL_TITLE);
	app.setApplicationDisplayName(QMIDICTL_TITLE);
	//	QMIDICTL_TITLE " - " + QObject::tr(QMIDICTL_SUBTITLE));
#endif

	qmidictlOptions opt;
	if (!opt.parse_args(app.arguments())) {
		app.quit();
		return 1;
	}

#if defined(Q_OS_ANDROID)

	// Custom color theme...
	QPalette pal(app.palette());
	update_palette(pal);
	app.setPalette(pal);

	// Custom style theme....
	QStyle *pAndroidStyle = QStyleFactory::create("Android");
	app.setStyle(new qmidictlActionBarStyle(pAndroidStyle));

	// Custom font size...
	const QFont& font = app.font();
	app.setFont(QFont(font.family(), font.pointSize() - 1));

#endif

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
#endif
#if defined(Q_OS_ANDROID) || defined(Q_OS_SYMBIAN)
	w.showMaximized();
#else
	w.show();
#endif
	w.setup();

	return app.exec();
}

// end of qmidictl.cpp

