#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

ORIGDIR=`pwd`
cd $srcdir

PROJECT=oaesis
FILE=src/lib/appl.c

(libtoolize --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have libtool installed to compile $PROJECT."
	echo "Download the appropriate package for your distribution,"
	echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
	DIE=1
}

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have autoconf installed to compile $PROJECT."
	echo "Download the appropriate package for your distribution,"
	echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
	DIE=1
}

(automake --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have automake installed to compile $PROJECT."
	echo "Download the appropriate package for your distribution,"
	echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
	DIE=1
}

(autoheader --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have autoheader installed to compile $PROJECT."
	echo "It is included in the autoconf package, available from"
	echo "your distribution, or from ftp://ftp.gnu.org/pub/gnu/"
	DIE=1
}

if test ! $TEST_TYPE $FILE
then
	echo "You must run this script in the top-level $PROJECT directory"
	exit 1
fi

if test "x$DIE" != "x"
then
    exit 1;
fi

echo -n "Running libtoolize... "
libtoolize --force
if test $? -ne 0; then echo; echo "libtoolize failed."; exit 1; fi
echo "done"

if test -d /usr/local/share/aclocal && test ! -h /usr/local/share/aclocal
then
    ACLOCAL_FLAGS="$ACLOCAL_FLAGS -I /usr/local/share/aclocal"
fi

echo -n "Running aclocal with options '$ACLOCAL_FLAGS'... "
aclocal $ACLOCAL_FLAGS
if test $? -ne 0; then echo; echo "aclocal failed."; exit 1; fi
echo "done"

# optionally feature autoheader
echo -n "Running autoheader... "
autoheader
if test $? -ne 0; then echo; echo "autoheader failed."; exit 1; fi
echo "done"

echo -n "Running automake... "
automake -a
if test $? -ne 0; then echo; echo "automake failed."; exit 1; fi
echo "done"

echo -n "Running autoconf... "
autoconf
if test $? -ne 0; then echo; echo "autoconf failed."; exit 1; fi
echo "done"

cd $ORIGDIR

$srcdir/configure "$@"
if test $? -ne 0; then echo; echo "configure failed."; exit 1; fi

if test $? -eq 0
then
    echo 
    echo "Now type 'make' to compile $PROJECT."
fi
