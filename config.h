/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* where to find the config files */
#define CONFIGDIR "/var/tuxbox/config"

/* where to find data */
#define DATADIR "/share/tuxbox"

/* Enable debug messages */
#define DEBUG 1

/* where to find the fonts */
#define FONTDIR "/share/fonts"

/* where games data is stored */
#define GAMESDIR "/var/tuxbox/games"

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the dvb includes */
#define HAVE_DVB 1

/* Define to the version of the dvb api */
#define HAVE_DVB_API_VERSION 3

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <linux/dvb/version.h> header file. */
/* #undef HAVE_LINUX_DVB_VERSION_H */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <ost/dmx.h> header file. */
/* #undef HAVE_OST_DMX_H */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* enable keyboard control, disable rc control */
/* #undef KEYBOARD_INSTEAD_OF_REMOTE_CONTROL */

/* where to find the internal libs */
#define LIBDIR "/lib/tuxbox"

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Name of package */
#define PACKAGE "tuxbox-neutrino"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "tuxbox-neutrino"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "tuxbox-neutrino 1.0.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "tuxbox-neutrino"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.0.1"

/* where to find the plugins */
#define PLUGINDIR "/lib/tuxbox/plugins"

/* enable return from graphics mode */
/* #undef RETURN_FROM_GRAPHICS_MODE */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* where to find the themes (don't change) */
#define THEMESDIR "/share/tuxbox/neutrino/themes"

/* where to find the ucodes */
#define UCODEDIR "/var/tuxbox/ucodes"

/* Version number of package */
#define VERSION "1.0.1"

/* Number of bits in a file offset, on hosts where this is settable. */
#define _FILE_OFFSET_BITS 64

/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */
