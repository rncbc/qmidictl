# qmidictl.pro
#
QMAKEVERSION = $$[QMAKE_VERSION]
ISQT4 = $$find(QMAKEVERSION, ^[2-9])
isEmpty( ISQT4 ) {
	error("Use the qmake include with Qt4.4 or greater, on Debian that is qmake-qt4");
}

TEMPLATE = subdirs
SUBDIRS = src

symbian {
        vendorinfo = "%{\"Pedro Lopez-Cabanillas\"}" ":\"Pedro Lopez-Cabanillas\""
        TARGET.UID3 = 0x20041DD8
}