
include_directories (
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${QT_INCLUDES}
)

link_directories (
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QT_LIBRARY_DIR}
)

configure_file (cmake_config.h ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set (ac_prefix ${CMAKE_INSTALL_PREFIX})

configure_file (qmidictl.desktop ${CMAKE_CURRENT_BINARY_DIR}/qmidictl.desktop)

set (HEADERS
  qmidictlAbout.h
  qmidictlOptions.h
  qmidictlMidiControl.h
  qmidictlMidiControlForm.h
  qmidictlOptionsForm.h
  qmidictlUdpDevice.h
  qmidictlMixerStrip.h
  qmidictlDialStyle.h
  qmidictlActionBar.h
  qmidictlActionBarStyle.h
  qmidictlMainForm.h
)

set (SOURCES
  qmidictl.cpp
  qmidictlOptions.cpp
  qmidictlOptionsForm.cpp
  qmidictlMidiControl.cpp
  qmidictlMidiControlForm.cpp
  qmidictlUdpDevice.cpp
  qmidictlMixerStrip.cpp
  qmidictlDialStyle.cpp
  qmidictlActionBar.cpp
  qmidictlActionBarStyle.cpp
  qmidictlMainForm.cpp
)

set (FORMS
  qmidictlOptionsForm.ui
  qmidictlMidiControlForm.ui
  qmidictlMixerStrip.ui
  qmidictlMainForm.ui
)

set (RESOURCES
  qmidictl.qrc
)

qt5_wrap_ui (UI_SOURCES ${FORMS})
qt5_wrap_cpp (MOC_SOURCES ${HEADERS})
qt5_add_resources (QRC_SOURCES ${RESOURCES})

add_executable (qmidictl
  ${UI_SOURCES}
  ${MOC_SOURCES}
  ${QRC_SOURCES}
  ${SOURCES}
)

target_link_libraries (qmidictl
  ${QT_LIBRARIES}
)

qt5_use_modules (qmidictl Core Gui Widgets Network)

if (UNIX AND NOT APPLE)
  install (TARGETS qmidictl
     RUNTIME DESTINATION bin)
  install (FILES ${CMAKE_CURRENT_BINARY_DIR}/qmidictl.desktop
     DESTINATION share/applications/hildon)
  install (FILES images/qmidictl.png
     DESTINATION share/icons/hicolor/32x32/hildon)
  install (FILES data/26x26/qmidictl.png
     DESTINATION share/icons/hicolor/26x26/hildon)
  install (FILES data/48x48/qmidictl.png
     DESTINATION share/icons/hicolor/48x48/hildon)
  install (FILES data/64x64/qmidictl.png
     DESTINATION share/icons/hicolor/64x64/hildon)
  install (FILES appdata/qmidictl.appdata.xml
     DESTINATION share/metainfo)
endif ()
