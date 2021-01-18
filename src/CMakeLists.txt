# project(qmidictl)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

configure_file (cmake_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

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

if (QT_VERSION VERSION_LESS 5.15.0)
  qt5_wrap_ui (UI_SOURCES ${FORMS})
  qt5_wrap_cpp (MOC_SOURCES ${HEADERS})
  qt5_add_resources (QRC_SOURCES ${RESOURCES})
else ()
  qt_wrap_ui (UI_SOURCES ${FORMS})
  qt_wrap_cpp (MOC_SOURCES ${HEADERS})
  qt_add_resources (QRC_SOURCES ${RESOURCES})
endif ()

if (WIN32)
  set (RC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/win32/${PROJECT_NAME}.rc)
  set (RES_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.res.obj)
  find_program (WINDRES_EXECUTABLE NAMES windres mingw32-windres i686-mingw32-windres)
  if (MINGW)
    exec_program (${WINDRES_EXECUTABLE}
      ARGS "-i ${RC_FILE} -o ${RES_FILE} --include-dir=${CMAKE_CURRENT_SOURCE_DIR}/images")
    list (APPEND SOURCES ${RES_FILE})
  else ()
    list (APPEND SOURCES ${RC_FILE})
  endif ()
endif ()

if (APPLE)
  set (ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/images/${PROJECT_NAME}.icns)
  list (APPEND SOURCES ${ICON_FILE})
  set (MACOSX_BUNDLE_ICON_FILE ${PROJECT_NAME}.icns)
  set_source_files_properties (${ICON_FILE} PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources)
endif ()


if (ANDROID)
  add_library (${PROJECT_NAME} SHARED
    ${UI_SOURCES}
    ${MOC_SOURCES}
    ${QRC_SOURCES}
    ${SOURCES}
  )
else ()
  add_executable (${PROJECT_NAME}
    ${UI_SOURCES}
    ${MOC_SOURCES}
    ${QRC_SOURCES}
    ${SOURCES}
  )
endif ()

set_target_properties (${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

if (WIN32)
  set_target_properties (${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE true)
endif ()

if (APPLE)
  set_target_properties (${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE true)
endif ()

target_link_libraries (${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if (CONFIG_IPV6)
  target_link_libraries (${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Network)
endif ()


if (UNIX AND NOT APPLE)
  install (TARGETS ${PROJECT_NAME} RUNTIME
     DESTINATION ${CMAKE_INSTALL_BINDIR})
  install (FILES ${PROJECT_NAME}.desktop
     DESTINATION ${CMAKE_INSTALL_DATADIR}/applications/hildon)
  install (FILES images/${PROJECT_NAME}.png
     DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/hildon)
  install (FILES data/26x26/${PROJECT_NAME}.png
     DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/26x26/hildon)
  install (FILES data/48x48/${PROJECT_NAME}.png
     DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/hildon)
  install (FILES data/64x64/${PROJECT_NAME}.png
     DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/hildon)
  install (FILES images/${PROJECT_NAME}.svg
     DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
  install (FILES appdata/${PROJECT_NAME}.appdata.xml
     DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
endif ()

if (WIN32)
   install (TARGETS ${PROJECT_NAME} RUNTIME
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
   install (FILES ${QM_FILES}
     DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/translations)
endif ()
