dnl $Id$
dnl config.m4 for extension wing

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(wing, for wing support,
dnl Make sure that the comment is aligned:
dnl [  --with-wing             Include wing support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(wing, whether to enable wing support,
dnl Make sure that the comment is aligned:
dnl [  --enable-wing           Enable wing support])

if test "$PHP_WING" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-wing -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/wing.h"  # you most likely want to change this
  dnl if test -r $PHP_WING/$SEARCH_FOR; then # path given as parameter
  dnl   WING_DIR=$PHP_WING
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for wing files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       WING_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$WING_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the wing distribution])
  dnl fi

  dnl # --with-wing -> add include path
  dnl PHP_ADD_INCLUDE($WING_DIR/include)

  dnl # --with-wing -> check for lib and symbol presence
  dnl LIBNAME=wing # you may want to change this
  dnl LIBSYMBOL=wing # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $WING_DIR/$PHP_LIBDIR, WING_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_WINGLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong wing lib version or lib not found])
  dnl ],[
  dnl   -L$WING_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(WING_SHARED_LIBADD)

  PHP_NEW_EXTENSION(wing, wing.c, $ext_shared)
fi
