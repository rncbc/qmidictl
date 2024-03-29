# project (qmidictl)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

set (CMAKE_AUTOUIC ON)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config.h)
  file (REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/config.h)
endif ()
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)


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

if (ANDROID AND QT_VERSION_MAJOR GREATER_EQUAL 6)
  qt_add_executable (${PROJECT_NAME} MANUAL_FINALIZATION
    ${HEADERS}
    ${SOURCES}
    ${FORMS}
    ${RESOURCES}
  )
else ()
  if (ANDROID)
    add_library (${PROJECT_NAME} SHARED
      ${HEADERS}
      ${SOURCES}
      ${FORMS}
      ${RESOURCES}
    )
  else ()
    add_executable (${PROJECT_NAME}
      ${HEADERS}
      ${SOURCES}
      ${FORMS}
      ${RESOURCES}
  )
  endif ()
endif ()

if (ANDROID)
  set_property(TARGET ${PROJECT_NAME}
    APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
      ${CMAKE_CURRENT_SOURCE_DIR}/android)
  string (REPLACE "." "0" VERSION_CODE "0${PROJECT_VERSION}")
  if (ANDROID_ABI MATCHES "x86_64")
     set (VERSION_CODE "${VERSION_CODE}04")
  elseif (ANDROID_ABI MATCHES "x86")
     set (VERSION_CODE "${VERSION_CODE}03")
  elseif (ANDROID_ABI MATCHES "armeabi-v7a")
     set (VERSION_CODE "${VERSION_CODE}02")
  elseif (ANDROID_ABI MATCHES "arm64-v8a")
     set (VERSION_CODE "${VERSION_CODE}01")
  else ()
     set (VERSION_CODE "${VERSION_CODE}00")
  endif ()
  set (VERSION_NAME "${PROJECT_VERSION}.${ANDROID_ABI}")
  set_property(TARGET ${PROJECT_NAME}
    APPEND PROPERTY QT_ANDROID_VERSION_CODE ${VERSION_CODE})
  set_property(TARGET ${PROJECT_NAME}
    APPEND PROPERTY QT_ANDROID_VERSION_NAME ${VERSION_NAME})
endif ()


# Add some debugger flags.
if (CONFIG_DEBUG AND UNIX AND NOT APPLE AND NOT ANDROID)
  set (CONFIG_DEBUG_OPTIONS -g -fsanitize=address -fno-omit-frame-pointer)
  target_compile_options (${PROJECT_NAME} PRIVATE ${CONFIG_DEBUG_OPTIONS})
  target_link_options (${PROJECT_NAME} PRIVATE ${CONFIG_DEBUG_OPTIONS})
endif ()

set_target_properties (${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

if (WIN32)
  set_target_properties (${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE true)
endif ()

if (APPLE)
  set_target_properties (${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE true)
endif ()

target_link_libraries (${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Svg)

if (CONFIG_IPV6)
  target_link_libraries (${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Network)
endif ()

if (UNIX AND NOT APPLE AND NOT ANDROID)
  install (TARGETS ${PROJECT_NAME} RUNTIME
    DESTINATION ${CMAKE_INSTALL_BINDIR})
  install (FILES images/${PROJECT_NAME}.png
    RENAME org.rncbc.${PROJECT_NAME}.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/apps)
  install (FILES images/${PROJECT_NAME}.svg
    RENAME org.rncbc.${PROJECT_NAME}.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
  install (FILES appdata/org.rncbc.${PROJECT_NAME}.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
  install (FILES appdata/org.rncbc.${PROJECT_NAME}.metainfo.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
  install (FILES man1/${PROJECT_NAME}.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
  install (FILES man1/${PROJECT_NAME}.fr.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/fr/man1 RENAME ${PROJECT_NAME}.1)
endif ()

if (WIN32)
  install (TARGETS ${PROJECT_NAME} RUNTIME
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  install (FILES ${QM_FILES}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/translations)
endif ()

if (ANDROID AND QT_VERSION_MAJOR GREATER_EQUAL 6)
  qt_finalize_executable (${PROJECT_NAME})
endif ()

