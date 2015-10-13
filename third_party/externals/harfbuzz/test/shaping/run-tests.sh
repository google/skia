#!/bin/sh

test "x$srcdir" = x && srcdir=.
test "x$builddir" = x && builddir=.
test "x$top_builddir" = x && top_builddir=../..

hb_shape=$top_builddir/util/hb-shape$EXEEXT

fails=0

if test $# = 0; then
	set /dev/stdin
fi

IFS=:
for f in "$@"; do
	echo "Running tests in $f"
	while read fontfile unicodes glyphs_expected; do
		echo "Testing $fontfile:$unicodes"
		glyphs=`$srcdir/hb-unicode-encode "$unicodes" | $hb_shape "$srcdir/$fontfile"`
		if ! test "x$glyphs" = "x$glyphs_expected"; then
			echo "Actual:   $glyphs" >&2
			echo "Expected: $glyphs_expected" >&2
			fails=$((fails+1))
		fi
	done < "$f"
done

if test $fails != 0; then
	echo "$fails tests failed."
	exit 1
else
	echo "All tests passed."
fi
