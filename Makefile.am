## Process this file with automake to produce Makefile.in

## --- Options ---

AUTOMAKE_OPTIONS = foreign

## --- What to build ---

lib_LIBRARIES = libcoopt.a

pkginclude_HEADERS = coopt.h

noinst_PROGRAMS = test

## --- Things to put in the library ---

libcoopt_a_SOURCES = coopt.c sopt.c

libcoopt_a_LIBADD = @LIBOBJS@

## --- Deps and links between library and test suite ---

DEPS = $(top_builddir)/libcoopt.a
LDADDS = $(top_builddir)/libcoopt.a

## --- Test suite ---

test_SOURCES = test.c
test_LDFLAGS = 
test_DEPENDENCIES = $(DEPS)
test_LDADD = $(LDADDS)

check: test
	./test
