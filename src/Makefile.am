## Process this file with automake to produce Makefile.in

if WITH_EDICTIDX
SUBDIRS = libedictidx libwaei
else
SUBDIRS = libwaei
endif

SUBDIRS += schemas

if WANT_WAEI
SUBDIRS += waei
endif

if WITH_GNOME
SUBDIRS += gwaei kpengine desktop images
endif

