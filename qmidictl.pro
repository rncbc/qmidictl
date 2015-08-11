# qmidictl.pro
#
TEMPLATE = subdirs
SUBDIRS = src

symbian {
        vendorinfo = "%{\"Pedro Lopez-Cabanillas\"}" ":\"Pedro Lopez-Cabanillas\""
        TARGET.UID3 = 0x20041DD8
}
