#! /bin/sh
# a quick hack script to generate necessary files from 
# auto* tools.
#
# WARNING: if you run this you will change the versions
# of the tools which are used and, maybe, required!

# You can define your own replacements in your environment.
# $AUTOCONF,  $AUTOMAKE, $AUTOHEADER, $AUTOPOINT, $ACLOCAL and $LIBTOOLIZE

touch Makefile.am configure.ac
{
   LT=${LIBTOOLIZE-libtoolize}
   echo "running $LT" >&2
   $LT --force --copy --automake
} && {
   AL=${ACLOCAL-aclocal}
   echo "running $AL" >&2
   $AL
} && {
   AH=${AUTOHEADER-autoheader}
   echo "running $AH [ignore the warnings]" >&2
   $AH
} && {
   AM=${AUTOMAKE-automake}
   echo "running $AM" >&2
   $AM --force-missing --foreign -a -c
} && {
   AC=${AUTOCONF-autoconf}
   echo "running $AC" >&2
   $AC
} &&
   echo "autogen complete" >&2 ||
   echo "ERROR: autogen.sh failed, autogen is incomplete" >&2
