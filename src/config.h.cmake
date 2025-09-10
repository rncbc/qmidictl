#ifndef CONFIG_H
#define CONFIG_H

/* Define to the title of this package. */
#cmakedefine PROJECT_TITLE "@PROJECT_TITLE@"

/* Define to the name of this package. */
#cmakedefine PROJECT_NAME "@PROJECT_NAME@"

/* Define to the version of this package. */
#cmakedefine PROJECT_VERSION "@PROJECT_VERSION@"

/* Define to the description of this package. */
#cmakedefine PROJECT_DESCRIPTION "@PROJECT_DESCRIPTION@"

/* Define to the homepage of this package. */
#cmakedefine PROJECT_HOMEPAGE_URL "@PROJECT_HOMEPAGE_URL@"

/* Define to the copyright of this package. */
#cmakedefine PROJECT_COPYRIGHT "@PROJECT_COPYRIGHT@"

/* Define to the domain of this package. */
#cmakedefine PROJECT_DOMAIN "@PROJECT_DOMAIN@"


/* Default installation prefix. */
#cmakedefine CONFIG_PREFIX "@CONFIG_PREFIX@"

/* Define to target installation dirs. */
#cmakedefine CONFIG_BINDIR "@CONFIG_BINDIR@"
#cmakedefine CONFIG_LIBDIR "@CONFIG_LIBDIR@"
#cmakedefine CONFIG_DATADIR "@CONFIG_DATADIR@"
#cmakedefine CONFIG_MANDIR "@CONFIG_MANDIR@"

/* Define if debugging is enabled. */
#cmakedefine CONFIG_DEBUG @CONFIG_DEBUG@

/* Define to 1 if you have the <signal.h> header file. */
#cmakedefine HAVE_SIGNAL_H @HAVE_SIGNAL_H@

/* Define if multi-touch is supported */
#cmakedefine CONFIG_MULTI_TOUCH @CONFIG_MULTI_TOUCH@

/* Define if IPv6 is supported */
#cmakedefine CONFIG_IPV6 @CONFIG_IPV6@


#endif /* CONFIG_H */
