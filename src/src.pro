# qmidictl.pro
#
TARGET = qmidictl

TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .

include(src.pri)

#DEFINES += DEBUG

HEADERS += config.h \
	qmidictlAbout.h \
	qmidictlOptions.h \
	qmidictlOptionsForm.h \
	qmidictlUdpDevice.h \
	qmidictlMixerStrip.h \
	qmidictlDialStyle.h \
	qmidictlMainForm.h

SOURCES += \
	qmidictl.cpp \
	qmidictlOptions.cpp \
	qmidictlOptionsForm.cpp \
	qmidictlUdpDevice.cpp \
	qmidictlMixerStrip.cpp \
	qmidictlDialStyle.cpp \
	qmidictlMainForm.cpp

FORMS += \
	qmidictlOptionsForm.ui \
	qmidictlMixerStrip.ui \
	qmidictlMainForm.ui

RESOURCES += qmidictl.qrc

unix {
	#VARIABLES
	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}
	BINDIR = $$PREFIX/bin
	DATADIR = $$PREFIX/share

	DEFINES += DATADIR=\"$$DATADIR\" PKGDATADIR=\"$$PKGDATADIR\"

	#MAKE INSTALL
	INSTALLS += target desktop iconxpm icon26 icon48 icon64

	target.path = $$BINDIR

	desktop.path = $$DATADIR/applications/hildon
	desktop.files += $${TARGET}.desktop

	iconxpm.path = $$DATADIR/pixmaps
	iconxpm.files += data/maemo/$${TARGET}.xpm

	icon26.path = $$DATADIR/icons/hicolor/26x26/apps
	icon26.files += data/26x26/$${TARGET}.png 

	icon40.path = $$DATADIR/icons/hicolor/48x48/apps
	icon40.files += data/48x48/$${TARGET}.png

	icon64.path = $$DATADIR/icons/hicolor/64x64/apps
	icon64.files += data/64x64/$${TARGET}.png
}

win32 {
	LIBS += -lwsock32
}
