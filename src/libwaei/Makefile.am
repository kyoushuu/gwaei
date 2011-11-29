
SUBDIRS = include/libwaei

LIBRARY_VERSION = 2:0:0
ACLOCAL_AMFLAGS = -I m4
datadir = @datadir@
DEFINITIONS =-DGZIP=\"$(GZIP)\" -DDATADIR2=\"$(datadir)\" -DGWAEI_LOCALEDIR=\"$(GWAEI_LOCALEDIR)\" -DG_SEAL_ENABLE -Wall 

lib_LTLIBRARIES = libwaei.la
libwaei_la_SOURCES = libwaei.c dictinfo.c dictinfolist.c dictinst.c dictinstlist.c queryline.c engine.c engine-data.c utilities.c io.c regex.c searchitem.c history.c resultline.c preferences.c vocabularylist.c vocabularyitem.c
libwaei_la_LDFLAGS = -version-info $(LIBRARY_VERSION)  $(LIBWAEI_LIBS) 
libwaei_la_CPPFLAGS = $(LIBWAEI_CFLAGS) -I$(top_srcdir)/src/libwaei/include $(DEFINITIONS) 

if WITH_EDICTIDX
libwaei_la_CPPFLAGS += -I$(top_srcdir)/src/libedictidx/ -DHAVE_EDICTIDX
libwaei_la_LDFLAGS += -L$(top_srcdir)/src/libedictidx/ -ledictidx
endif

if OS_MINGW
libwaei_la_LDFLAGS +=-Wl,-subsystem,windows 
MINGW_DEFS =-DCURL_STATICLIB 
libwaei_la_LDFLAGS +=$(MINGW_LIBS) 
libwaei_la_CPPFLAGS +=$(MINGW_CFLAGS) $(MINGW_DEFS)
endif


