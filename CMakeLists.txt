cmake_minimum_required (VERSION 3.13)

project (qmidictl
  VERSION 0.9.4
  DESCRIPTION "A MIDI Remote Controller via UDP/IP Multicast"
  LANGUAGES C CXX)

set (CONFIG_VERSION ${PROJECT_VERSION})
execute_process (
  COMMAND git describe --tags --dirty --abbrev=6
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_DESCRIBE_OUTPUT
  RESULT_VARIABLE GIT_DESCRIBE_RESULT
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if (GIT_DESCRIBE_RESULT EQUAL 0)
  set (VERSION "${GIT_DESCRIBE_OUTPUT}")
  string (REGEX REPLACE "^[^0-9]+" "" VERSION "${VERSION}")
  string (REGEX REPLACE "^1_"      "" VERSION "${VERSION}")
  string (REGEX REPLACE "^[_vV]+"  "" VERSION "${VERSION}")
  string (REGEX REPLACE "-g"   "git." VERSION "${VERSION}")
  string (REGEX REPLACE "[_|-]"   "." VERSION "${VERSION}")
  execute_process (
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_REVPARSE_OUTPUT
    RESULT_VARIABLE GIT_REVPARSE_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (GIT_REVPARSE_RESULT EQUAL 0 AND NOT GIT_REVPARSE_OUTPUT STREQUAL "master")
    set (VERSION "${VERSION} [${GIT_REVPARSE_OUTPUT}]")
  endif ()
else ()
  set (VERSION "${PROJECT_VERSION}")
endif ()

set (PACKAGE_NAME "QmidiCtl")
set (PACKAGE_VERSION "${VERSION}")
set (PACKAGE_BUGREPORT "rncbc@rncbc.org")
set (PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set (PACKAGE_TARNAME "qmidictl")

set (ac_version "${PROJECT_VERSION}")
set (ac_prefix "${CMAKE_INSTALL_PREFIX}")

set (CONFIG_BUILD_VERSION "${PACKAGE_VERSION}")

if (CONFIG_DEBUG)
  set (CMAKE_BUILD_TYPE "Debug")
endif ()
if (CMAKE_BUILD_TYPE MATCHES "Debug")
  set (CONFIG_DEBUG 1)
endif ()
if (CONFIG_DEBUG)
  set (CONFIG_BUILD_TYPE "debug")
else ()
  set (CONFIG_BUILD_TYPE "release")
endif ()

set (CONFIG_PREFIX "${CMAKE_INSTALL_PREFIX}")

include (GNUInstallDirs)
set (CONFIG_BINDIR  "${CONFIG_PREFIX}/${CMAKE_INSTALL_BINDIR}")
set (CONFIG_LIBDIR  "${CONFIG_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
set (CONFIG_DATADIR "${CONFIG_PREFIX}/${CMAKE_INSTALL_DATADIR}")
set (CONFIG_MANDIR  "${CONFIG_PREFIX}/${CMAKE_INSTALL_MANDIR}")


# Enable Network IPv6 support option.
option (CONFIG_IPV6 "Enable Network IPv6 support (EXPERIMENTAL) (default=yes)" 1)


# Enable Qt6 build preference.
option (CONFIG_QT6 "Enable Qt6 build (default=yes)" 1)


# Fix for new CMAKE_REQUIRED_LIBRARIES policy.
if (POLICY CMP0075)
  cmake_policy (SET CMP0075 NEW)
endif ()

# Check for Qt...
if (CONFIG_QT6)
  find_package (Qt6 QUIET)
  if (NOT Qt6_FOUND)
    set (CONFIG_QT6 0)
  endif ()
endif ()

if (CONFIG_QT6)
  find_package (QT QUIET NAMES Qt6)
else ()
  find_package (QT QUIET NAMES Qt5)
endif ()

find_package (Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)

if (CONFIG_IPV6)
  find_package (Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network)
endif ()

#find_package (Qt${QT_VERSION_MAJOR}LinguistTools)

include (CheckIncludeFile)
include (CheckIncludeFiles)
include (CheckIncludeFileCXX)
include (CheckFunctionExists)
include (CheckLibraryExists)

# Checks for header files.
if (UNIX AND NOT APPLE)
  check_include_files ("fcntl.h;unistd.h;signal.h" HAVE_SIGNAL_H)
endif ()


add_subdirectory (src)

configure_file (qmidictl.spec.in qmidictl.spec IMMEDIATE @ONLY)

install (FILES qmidictl.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
install (FILES qmidictl.fr.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/fr/man1 RENAME qmidictl.1)

# Configuration status
macro (SHOW_OPTION text value)
  if (${value})
    message ("${text}: yes")
  else ()
    message ("${text}: no")
  endif ()
endmacro ()

message   ("\n  ${PACKAGE_NAME} ${PACKAGE_VERSION} (Qt ${QT_VERSION})")
message   ("\n  Build target . . . . . . . . . . . . . . . . . . .: ${CONFIG_BUILD_TYPE}\n")
show_option ("  Network IPv6 support (EXPERIMENTAL)  . . . . . . ." CONFIG_IPV6)
message   ("\n  Install prefix . . . . . . . . . . . . . . . . . .: ${CONFIG_PREFIX}")
message   ("\nNow type 'make', followed by 'make install' as root.\n")
