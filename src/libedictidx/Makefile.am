noinst_LTLIBRARIES = libedictidx.la

SRCS = edictidx.h edictidx_int.h fileops.c hash.c key.c parser.c query.c build.c verify.c edictidx.c

if HAVE_MMAP
SRCS += mmap-posix.c
else
SRCS += mmap-stdio.c
endif

libedictidx_la_SOURCES = $(SRCS)

check_PROGRAMS = tests/hash tests/parser tests/build tests/verify tests/query

LDADD = libedictidx.la

tests_hash_SOURCES = tests/tests.c
tests_hash_CPPFLAGS = -DHASH_TEST

tests_parser_SOURCES = tests/tests.c
tests_parser_CPPFLAGS = -DPARSER_TEST

tests_build_SOURCES = tests/tests.c
tests_build_CPPFLAGS = -DBUILD_TEST

tests_verify_SOURCES = tests/tests.c
tests_verify_CPPFLAGS = -DVERIFY_TEST

tests_query_SOURCES = tests/tests.c
tests_query_CPPFLAGS = -DQUERY_TEST

.PHONY: splint

SPLINT = splint
SPLINT_FLAGS = -nolib +quiet +strict \
	+partial -exportheader +neverinclude \
	-protoparamname \
	-boundsread -boundswrite \
	+ptrnegate -predboolptr \
	-ifblock \
	-modfilesys
SPLINT_DEFS =

splint: splint_library.h $(SRCS)
	$(SPLINT) $(SPLINT_FLAGS) $(SPLINT_DEFS) $^

