## Process this file with automake to produce Makefile.in

## --- Options ---

AUTOMAKE_OPTIONS = foreign

## --- What to build ---

lib_LIBRARIES = libcoopt.a

## --- Things to install ---

include_HEADERS = coopt.h

man_MANS = coopt.3
#man_MANS = coopt.3 coopt_serror.3 coopt_sopt.3

m4datadir = $(datadir)/aclocal
m4data_DATA = libcoopt.m4

## --- Things to put in the library ---

libcoopt_a_SOURCES = coopt.c sopt.c serror.c

libcoopt_a_LIBADD = @LIBOBJS@

## --- Deps and links between library and test suite ---

DEPS = $(top_builddir)/libcoopt.a
LDADDS = $(top_builddir)/libcoopt.a

## --- Test suite ---

check_PROGRAMS = test
test_SOURCES = test.c
test_LDFLAGS = 
test_DEPENDENCIES = $(DEPS)
test_LDADD = $(LDADDS)

TESTS = test
