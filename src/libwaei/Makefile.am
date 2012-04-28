
SUBDIRS = include/libwaei

LIBRARY_VERSION = 2:0:0
ACLOCAL_AMFLAGS = -I m4
datadir = @datadir@
DEFINITIONS =-DDATADIR2=\"$(datadir)\" -DGWAEI_LOCALEDIR=\"$(GWAEI_LOCALEDIR)\" -DG_SEAL_ENABLE -Wall 

lib_LTLIBRARIES =libwaei.la
libwaei_la_SOURCES =libwaei.c dictionary.c edictionary.c kanjidictionary.c exampledictionary.c unknowndictionary.c installdictionary.c installdictionarylist.c dictionarylist.c query.c utilities.c io.c regex.c regexgroup.c search.c history.c result.c preferences.c vocabularylist.c vocabularyitem.c
libwaei_la_LDFLAGS =-version-info $(LIBRARY_VERSION)  $(LIBWAEI_LIBS) $(MECAB_LIBS)
libwaei_la_CPPFLAGS = $(LIBWAEI_CFLAGS) -I$(top_srcdir)/src/libwaei/include $(DEFINITIONS) 

if WITH_MECAB
MECAB_DEFS =-DWITH_MECAB
libwaei_la_SOURCES += morphology.c
libwaei_la_CPPFLAGS +=$(MECAB_DEFS)
endif

if OS_MINGW
libwaei_la_LDFLAGS +=-Wl,-subsystem,windows 
MINGW_DEFS =-DCURL_STATICLIB 
libwaei_la_LDFLAGS +=$(MINGW_LIBS)
libwaei_la_CPPFLAGS +=$(MINGW_CFLAGS) $(MINGW_DEFS)
endif


