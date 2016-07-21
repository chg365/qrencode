dnl $Id$
dnl config.m4 for extension qrencode

PHP_ARG_WITH(qrencode, for qrencode support,
Make sure that the comment is aligned:
[  --with-qrencode             Include qrencode support])

if test "$PHP_QRENCODE" != "no"; then
  # --with-qrencode -> check with-path
  SEARCH_PATH="/usr/local /usr"     # you might want to change this
  SEARCH_FOR="/include/qrencode.h"  # you most likely want to change this
  if test -r $PHP_QRENCODE/$SEARCH_FOR; then # path given as parameter
    QRENCODE_DIR=$PHP_QRENCODE
  else # search default path list
    AC_MSG_CHECKING([for qrencode files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        QRENCODE_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$QRENCODE_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the qrencode distribution])
  fi

  PHP_ADD_INCLUDE($QRENCODE_DIR/include)

  dnl # --with-qrencode -> check for lib and symbol presence
  dnl LIBNAME=qrencode # you may want to change this
  dnl LIBSYMBOL=qrencode # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $QRENCODE_DIR/$PHP_LIBDIR, QRENCODE_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_QRENCODELIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong qrencode lib version or lib not found])
  dnl ],[
  dnl   -L$QRENCODE_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  PHP_SUBST(QRENCODE_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(png, $QRENCODE_DIR/$PHP_LIBDIR, QRENCODE_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(qrencode, $QRENCODE_DIR/$PHP_LIBDIR, QRENCODE_SHARED_LIBADD)
  PHP_NEW_EXTENSION(qrencode, qrencode.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
