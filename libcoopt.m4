# a macro to get the libs/cflags for libcoopt
# serial 1

dnl AM_PATH_LIBCOOPT([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Find paths to LIBCOOPT
dnl Defines LIBCOOPT_CFLAGS and LIBCOOPT_LIBS
dnl
AC_DEFUN(AM_PATH_LIBCOOPT,
[dnl
dnl Use supplied paths first, otherwise:
dnl Check in /usr/local/include, /usr/include, coopt and libcoopt for coopt.h
dnl Check in /usr/local/lib, /usr/lib, and coopt and libcoopt for libcoopt.a
dnl

AC_ARG_WITH(libcoopt-prefix,
[  --with-libcoopt-prefix  Prefix under which libcoopt was installed],
LIBCOOPT_PREFIX="$withval")

AC_ARG_WITH(libcoopt-include,
[  --with-libcoopt-include Include directory for coopt header file],
LIBCOOPT_INCLUDE="$withval")

AC_ARG_WITH(libcoopt-exec,
[  --with-libcoopt-exec    Directory containing coopt library],
LIBCOOPT_EXEC="$withval")


if test "x$LIBCOOPT_PREFIX" = "xyes"; then
  AC_MSG_ERROR(--with-libcoopt-prefix needs prefix under which libcoopt was installed)
fi

if test "x$LIBCOOPT_INCLUDE" = "xyes"; then
  AC_MSG_ERROR(--with-libcoopt-include needs include directory for coopt header file)
fi

if test "x$LIBCOOPT_EXEC" = "xyes"; then
  AC_MSG_ERROR(--with-libcoopt-exec needs directory containing coopt library)
fi


AC_MSG_CHECKING(for libcoopt)

LIBCOOPT_INCLUDE_PATH=
LIBCOOPT_EXEC_PATH=

dnl Use parameters if specified
dnl Prefix: specifies both paths
if test "x$LIBCOOPT_PREFIX" != "x"; then
  LIBCOOPT_INCLUDE_PATH=$LIBCOOPT_PREFIX/include
  LIBCOOPT_EXEC_PATH=$LIBCOOPT_PREFIX/lib
fi
dnl Include - overrides prefix
if test "x$LIBCOOPT_INCLUDE" != "x"; then
  LIBCOOPT_INCLUDE_PATH=$LIBCOOPT_INCLUDE
  if test "x$LIBCOOPT_EXEC" != "x" && test "x$LIBCOOPT_PREFIX" != "x"; then
    AC_MSG_WARN(--libcoopt-prefix parameter entirely overridden)
  fi
fi
dnl Exec - overrides prefix
if test "x$LIBCOOPT_EXEC" != "x"; then
  LIBCOOPT_EXEC_PATH=$LIBCOOPT_EXEC
fi

dnl If not specified yet, look in /usr/local
if test "x$LIBCOOPT_INCLUDE_PATH" = "x"; then
  if test -r "/usr/local/include/coopt.h"; then
    LIBCOOPT_INCLUDE_PATH=/usr/local/include
  fi
fi
if test "x$LIBCOOPT_EXEC_PATH" = "x"; then
  if test -r "/usr/local/lib/libcoopt.a" ||
     test -r "/usr/local/lib/libcoopt.so"; then
    LIBCOOPT_EXEC_PATH=/usr/local/lib
  fi
fi

dnl If still not specified, look in /usr
if test "x$LIBCOOPT_INCLUDE_PATH" = "x"; then
  if test -r "/usr/include/coopt.h"; then
    LIBCOOPT_INCLUDE_PATH=/usr/include
  fi
fi
if test "x$LIBCOOPT_EXEC_PATH" = "x"; then
  if test -r "/usr/lib/libcoopt.a" ||
     test -r "/usr/lib/libcoopt.so"; then
    LIBCOOPT_EXEC_PATH=/usr/lib
  fi
fi

dnl If still not specified, look in `pwd`/coopt
CURRENT_DIR=`pwd`
if test "x$LIBCOOPT_INCLUDE_PATH" = "x"; then
  if test -r "$CURRENT_DIR/coopt/coopt.h"; then
    LIBCOOPT_INCLUDE_PATH=$CURRENT_DIR/coopt
  fi
fi
if test "x$LIBCOOPT_EXEC_PATH" = "x"; then
  if test -r "$CURRENT_DIR/coopt/libcoopt.a" ||
     test -r "$CURRENT_DIR/coopt/libcoopt.so"; then
    LIBCOOPT_EXEC_PATH=$CURRENT_DIR/coopt
  fi
fi

dnl If still not specified, finally look in `pwd`/libcoopt
CURRENT_DIR=`pwd`
if test "x$LIBCOOPT_INCLUDE_PATH" = "x"; then
  if test -r "$CURRENT_DIR/libcoopt/coopt.h"; then
    LIBCOOPT_INCLUDE_PATH=$CURRENT_DIR/libcoopt
  fi
fi
if test "x$LIBCOOPT_EXEC_PATH" = "x"; then
  if test -r "$CURRENT_DIR/libcoopt/libcoopt.a" ||
     test -r "$CURRENT_DIR/libcoopt/libcoopt.so"; then
    LIBCOOPT_EXEC_PATH=$CURRENT_DIR/libcoopt
  fi
fi

echo "LIBCOOPT_INCLUDE_PATH=$LIBCOOPT_INCLUDE_PATH"
echo "LIBCOOPT_EXEC_PATH=$LIBCOOPT_EXEC_PATH"

dnl Check files exist - if either doesn't, unset both variables
if test -r "$LIBCOOPT_INCLUDE_PATH/coopt.h"; then
  :
else
   if test -r "$LIBCOOPT_EXEC_PATH/libcoopt.a" ||
      test -r "$LIBCOOPT_EXEC_PATH/libcoopt.so"; then
     :
   else
     LIBCOOPT_INCLUDE_PATH=
     LIBCOOPT_EXEC_PATH=
   fi
fi

dnl If not specified (or didn't exist), give a warning
if test "x$LIBCOOPT_INCLUDE_PATH" = "x" || "x$LIBCOOPT_EXEC_PATH" = "x"; then
  AC_MSG_WARN(Couldn't find coopt)
fi

dnl Define flags with specified paths
LIBCOOPT_CFLAGS="-I$LIBCOOPT_INCLUDE_PATH"
LIBCOOPT_LIBS="-L$LIBCOOPT_EXEC_PATH -lcoopt"

AC_SUBST(LIBCOOPT_CFLAGS)
AC_SUBST(LIBCOOPT_LIBS)
])
