dnl $Id$
dnl config.m4 for extension wing_encrypt

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(wing_encrypt, for wing_encrypt support,
dnl Make sure that the comment is aligned:
dnl [  --with-wing_encrypt             Include wing_encrypt support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(wing_encrypt, whether to enable wing_encrypt support,
dnl Make sure that the comment is aligned:
dnl [  --enable-wing_encrypt           Enable wing_encrypt support])

if test "$PHP_WING_ENCRYPT" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-wing_encrypt -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/wing_encrypt.h"  # you most likely want to change this
  dnl if test -r $PHP_WING_ENCRYPT/$SEARCH_FOR; then # path given as parameter
  dnl   WING_ENCRYPT_DIR=$PHP_WING_ENCRYPT
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for wing_encrypt files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       WING_ENCRYPT_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$WING_ENCRYPT_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the wing_encrypt distribution])
  dnl fi

  dnl # --with-wing_encrypt -> add include path
  dnl PHP_ADD_INCLUDE($WING_ENCRYPT_DIR/include)

  dnl # --with-wing_encrypt -> check for lib and symbol presence
  dnl LIBNAME=wing_encrypt # you may want to change this
  dnl LIBSYMBOL=wing_encrypt # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $WING_ENCRYPT_DIR/$PHP_LIBDIR, WING_ENCRYPT_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_WING_ENCRYPTLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong wing_encrypt lib version or lib not found])
  dnl ],[
  dnl   -L$WING_ENCRYPT_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(WING_ENCRYPT_SHARED_LIBADD)

  PHP_NEW_EXTENSION(wing_encrypt, wing_encrypt.c, $ext_shared)
fi
