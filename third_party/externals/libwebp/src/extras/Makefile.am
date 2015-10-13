lib_LTLIBRARIES = libwebpextras.la

libwebpextras_la_SOURCES =
libwebpextras_la_SOURCES += extras.c

libwebpextrasinclude_HEADERS =
libwebpextrasinclude_HEADERS += ../webp/extras.h
libwebpextrasinclude_HEADERS += ../webp/types.h

libwebpextras_la_LIBADD = ../libwebp.la
libwebpextras_la_LDFLAGS = -no-undefined -version-info 0:0:0
libwebpextrasincludedir = $(includedir)/webp
pkgconfig_DATA = libwebpextras.pc
