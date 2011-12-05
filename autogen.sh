#!/bin/sh

echo regenerating ./src/libedictidx/
( cd ./src/libedictidx ; ./autogen.sh )

libtoolize --install --copy --force \
&& gettextize --force --no-changelog \
&& intltoolize --copy --force --automake \
&& mv -f configure.ac~ configure.ac \
; mv -f Makefile.am~ Makefile.am \
; true \
&& gnome-doc-prepare --copy --force \
&& aclocal -I m4 --force \
&& autoheader -f \
&& automake --copy --force --add-missing \
&& autoconf --force

#this line should go after gettextize
