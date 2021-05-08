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
	qmidictlMidiControl.h \
	qmidictlMidiControlForm.h \
	qmidictlOptionsForm.h \
	qmidictlUdpDevice.h \
	qmidictlMixerStrip.h \
	qmidictlDialStyle.h \
	qmidictlActionBar.h \
	qmidictlActionBarStyle.h \
	qmidictlMainForm.h

SOURCES += \
	qmidictl.cpp \
	qmidictlOptions.cpp \
	qmidictlOptionsForm.cpp \
	qmidictlMidiControl.cpp \
	qmidictlMidiControlForm.cpp \
	qmidictlUdpDevice.cpp \
	qmidictlMixerStrip.cpp \
	qmidictlDialStyle.cpp \
	qmidictlActionBar.cpp \
	qmidictlActionBarStyle.cpp \
	qmidictlMainForm.cpp

FORMS += \
	qmidictlOptionsForm.ui \
	qmidictlMidiControlForm.ui \
	qmidictlMixerStrip.ui \
	qmidictlMainForm.ui

RESOURCES += qmidictl.qrc

unix {

	# variables
	OBJECTS_DIR = .obj
	MOC_DIR     = .moc
	UI_DIR      = .ui

	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}

	isEmpty(BINDIR) {
		BINDIR = $${PREFIX}/bin
	}

	isEmpty(DATADIR) {
		DATADIR = $${PREFIX}/share
	}

	#DEFINES += DATADIR=\"$${DATADIR}\"
	DEFINES += PKGDATADIR=\"$${PKGDATADIR}\"

	# make install
	INSTALLS += target desktop icon icon26 icon48 icon64 icon_scalable appdata

	target.path = $${BINDIR}

	desktop.path = $${DATADIR}/applications/hildon
	desktop.files += $${TARGET}.desktop

	icon.path = $${DATADIR}/icons/hicolor/32x32/hildon
	icon.files += images/$${TARGET}.png

	icon26.path = $${DATADIR}/icons/hicolor/26x26/hildon
	icon26.files += data/26x26/$${TARGET}.png 

	icon48.path = $${DATADIR}/icons/hicolor/48x48/hildon
	icon48.files += data/48x48/$${TARGET}.png

	icon64.path = $${DATADIR}/icons/hicolor/64x64/hildon
	icon64.files += data/64x64/$${TARGET}.png

	icon_scalable.path = $${DATADIR}/icons/hicolor/scalable/apps
	icon_scalable.files += images/$${TARGET}.svg

	appdata.path = $${DATADIR}/metainfo
	appdata.files += appdata/$${TARGET}.appdata.xml
}

QT += widgets

win32 {
	LIBS += -lwsock32
}

symbian {
	LIBS += -lcone -leikcore -lavkon
	ICON += data/symbian/qmidictl.svg
	TARGET.CAPABILITY += NetworkServices LocalServices
}

android {

	# variables
	OBJECTS_DIR = .obj_$${ANDROID_TARGET_ARCH}
	MOC_DIR     = .moc_$${ANDROID_TARGET_ARCH}
	UI_DIR      = .ui_$${ANDROID_TARGET_ARCH}

	defineReplace(androidVersionCode) {
		vlist = $$split(1, ".")
		for (vitem, vlist): \
			vcode = "$$first(vcode)$$format_number($$vitem, width=2 zeropad)"
		varch = 00
		return($$first(vcode)$$first(varch))
	}

	ANDROID_VERSION_NAME = $${VERSION}
	ANDROID_VERSION_CODE = $$androidVersionCode($${ANDROID_VERSION_NAME})

	DISTFILES += \
		android/AndroidManifest.xml \
		android/build.gradle \
		android/res/values/libs.xml
	ANDROID_PACKAGE_SOURCE_DIR = $${PWD}/android
}

ANDROID_ABIS = armeabi-v7a arm64-v8a x86 x86_64
