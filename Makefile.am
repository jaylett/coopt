## Process this file with automake to produce Makefile.in

## --- Options ---

AUTOMAKE_OPTIONS = foreign

## --- What to build ---

lib_LIBRARIES = libcoopt.a

pkginclude_HEADERS = coopt.h

## --- Things to put in the library ---

libcoopt_a_SOURCES = coopt.c sopt.c

libcoopt_a_LIBADD = @LIBOBJS@

## --- Test suite ---

test: test.o libcoopt.a
	$(LINK) test.o libcoopt.a
	./test
