lib_LTLIBRARIES = liblsrt.la

liblsrt_la_CPPFLAGS = -W -Wall -Wstrict-prototypes -I$(top_srcdir)/src/include -I$(srcdir)/include
liblsrt_la_LDFLAGS = -lgmp @TARGET_LIBS@
liblsrt_la_SOURCES = utils.c \
                     number.c \
                     strings.c \
                     pairlist.c \
                     io.c

noinst_HEADERS = ../include/runtime/object.h \
                 include/utils.h \
                 include/number.h
