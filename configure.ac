# Process this file with autoconf to produce a configure script.
AC_INIT(QmidiCtl, 0.2.0.2, rncbc@rncbc.org, qmidictl)

AC_CONFIG_SRCDIR(src/qmidictl.cpp)
AC_CONFIG_HEADERS(src/config.h)
AC_CONFIG_FILES(Makefile qmidictl.spec src/src.pri src/qmidictl.desktop)

# Set default installation prefix.
AC_PREFIX_DEFAULT(/usr/local)
ac_prefix=$prefix
if test "x$ac_prefix" = "xNONE"; then
    ac_prefix=$ac_default_prefix
fi
AC_SUBST(ac_prefix)
AC_DEFINE_UNQUOTED(CONFIG_PREFIX, ["$ac_prefix"], [Default installation prefix.])

# Enable debugging argument option.
AC_ARG_ENABLE(debug,
  AC_HELP_STRING([--enable-debug], [enable debugging (default=no)]),
  [ac_debug="$enableval"])

if test "x$ac_debug" = "xyes"; then
   AC_DEFINE(CONFIG_DEBUG, 1, [Define if debugging is enabled.])
   ac_debug="debug"
else
   ac_debug="release"
fi

AC_SUBST(ac_debug)


# Enable Qt4/5 availability.
AC_ARG_ENABLE(qt4,
  AC_HELP_STRING([--enable-qt4], [enable Qt4 build (default=yes)]),
  [ac_qt4="$enableval"],
  [ac_qt4="yes"])

AC_ARG_ENABLE(qt5,
  AC_HELP_STRING([--enable-qt5], [enable Qt5 build (default=no)]),
  [ac_qt5="$enableval"],
  [ac_qt5="no"])



# Standard installation base dirs.
ac_with_paths="/usr /usr/local"

# Set for alternate Qt4/5 installation dir.
AC_ARG_WITH(qt4,
  AC_HELP_STRING([--with-qt4=PATH], [use alternate Qt4 install path]),
  [ac_qt4_path="$withval"], [ac_qt4_path="no"])

AC_ARG_WITH(qt5,
  AC_HELP_STRING([--with-qt5=PATH], [use alternate Qt5 install path]),
  [ac_qt5_path="$withval"], [ac_qt5_path="no"])

if test "x$ac_qt4_path" != "xno"; then
   ac_with_paths="$ac_with_paths $ac_qt4_path"
   ac_qt4="yes"
fi

if test "x$ac_qt5_path" != "xno"; then
   ac_with_paths="$ac_with_paths $ac_qt5_path"
   ac_qt5="yes"
fi

if test "x$ac_qt4" = "xno"; then
   ac_qt5="yes"
fi
if test "x$ac_qt5" = "xyes"; then
   ac_qt4="no"
fi


# Honor user specified flags.
ac_cflags=$CFLAGS
ac_ldflags=$LDFLAGS


# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_GCC_TRADITIONAL

# Checks for languages.
AC_LANG_C
AC_LANG_CPLUSPLUS


# Check for proper flags.
ac_arch=`uname -m`

# Check for some a-la-debian alternatives...
ac_qtdirs="qt"

if test "x$ac_qt4" = "xyes"; then
   ac_qtdirs="qt4 $ac_qtdirs"
fi
if test "x$ac_qt5" = "xyes"; then
   ac_qtdirs="qt5 $ac_qtdirs"
fi

ac_topdirs="/usr/share /usr/lib"

if test "x$ac_arch" = "xx86_64"; then
   ac_topdirs="$ac_topdirs /usr/lib64"
fi

if test "x$ac_arch" = "xx86_64" -o "x$ac_qt5" = "xyes"; then
   CFLAGS="-fPIC $CFLAGS"
   CPPFLAGS="-fPIC $CPPFLAGS"
fi

for X in $ac_topdirs; do
  for Y in $ac_qtdirs; do
    if test -d $X/$Y/bin; then
      ac_with_paths="$X/$Y $ac_with_paths"
    fi
  done
done

# Prepend alternate dependencies paths.
ac_path=$PATH
for X in $ac_with_paths; do
  if test -d $X/bin; then
    ac_path="$X/bin:$ac_path"
  fi
# if test -x $X/qmake; then
#   ac_path="$X:$ac_path"
# fi
  if test -d $X/include; then
    for Y in $ac_qtdirs; do
      if test -d $X/include/$Y; then
        CFLAGS="-I$X/include/$Y $CFLAGS"
        CPPFLAGS="-I$X/include/$Y $CPPFLAGS"
        ac_incpath="$X/include/$Y $ac_incpath"
      fi
    done
    CFLAGS="$CFLAGS -I$X/include"
    CPPFLAGS="$CPPFLAGS -I$X/include"
    ac_incpath="$ac_incpath $X/include"
  fi
  if test "x$ac_arch" = "xx86_64" -a -d $X/lib64; then
    LIBS="-L$X/lib64 $LIBS"
    ac_libs="-L$X/lib64 $ac_libs"
  fi
  if test -d $X/lib; then
    LIBS="-L$X/lib $LIBS"
    ac_libs="-L$X/lib $ac_libs"
  fi
done


# Check for proper Qt4/5 version.
if test "x$ac_qt4" = "xyes"; then
   AC_CACHE_CHECK([for Qt library version >= 4.4],
      ac_cv_qtversion, [
      AC_TRY_LINK([#include "QtCore/qglobal.h"], [
         #if QT_VERSION < 0x040400 || QT_VERSION >= 0x050000
         #error Qt library 4.4 or greater required.
         #endif
      ], ac_cv_qtversion="yes", [
         echo "no; Qt 4.4 or greater is required"
         exit 1
      ])
   ])
fi

if test "x$ac_qt5" = "xyes"; then
   AC_CACHE_CHECK([for Qt library version >= 5.1],
      ac_cv_qtversion, [
      AC_TRY_LINK([#include "QtCore/qglobal.h"], [
         #if QT_VERSION < 0x050100 || QT_VERSION >= 0x060000
         #error Qt library 5.1 or greater required.
         #endif
      ], ac_cv_qtversion="yes", [
         echo "no; Qt 5.1 or greater is required"
         exit 1
      ])
   ])
fi

# A common error message:
ac_errmsg="not found in current PATH. Maybe QT development environment isn't available (qt-devel)."

# Check for Qt qmake utility.
AC_PATH_PROG(ac_qmake, qmake, [no], $ac_path)
if test "x$ac_qmake" = "xno"; then
   AC_MSG_ERROR([qmake $ac_errmsg])
fi
AC_SUBST(ac_qmake)

# Check for Qt moc utility.
AC_PATH_PROG(ac_moc, moc, [no], $ac_path)
if test "x$ac_moc" = "xno"; then
   AC_MSG_ERROR([moc $ac_errmsg])
fi
AC_SUBST(ac_moc)

# Check for Qt uic utility.
AC_PATH_PROG(ac_uic, uic, [no], $ac_path)
if test "x$ac_uic" = "xno"; then
   AC_MSG_ERROR([uic $ac_errmsg])
fi
AC_SUBST(ac_uic)


# Checks for local libraries.
AC_CHECK_LIB(m, main)
AC_CHECK_LIB(X11, main)
AC_CHECK_LIB(Xext, main)

# Check for specific libraries.
AC_SUBST(ac_libs)


# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h sys/ioctl.h sys/stat.h unistd.h signal.h)

# Check for specific headers.
AC_SUBST(ac_incpath)
AC_SUBST(ac_cflags)
AC_SUBST(ac_ldflags)


# Checks for typedefs, structures, and compiler characteristics.
# AC_C_CONST

# Finally produce a configure header file and the makefiles.
AC_OUTPUT

# make clean > /dev/null 2>&1

# Output summary message

echo
echo "  $PACKAGE_NAME $PACKAGE_VERSION"
echo
echo "  Build target . . . . . . . . . . . . . . . . . . .: $ac_debug"
echo
echo "  Install prefix . . . . . . . . . . . . . . . . . .: $ac_prefix"
echo
echo "Now type 'make', followed by 'make install' as root."
echo
