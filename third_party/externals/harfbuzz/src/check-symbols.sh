#!/bin/sh

LC_ALL=C
export LC_ALL

test -z "$srcdir" && srcdir=.
stat=0


if which nm 2>/dev/null >/dev/null; then
	:
else
	echo "check-symbols.sh: 'nm' not found; skipping test"
	exit 77
fi

echo "Checking that we are not exposing internal symbols"
tested=false
for so in `ls .libs/lib*.so .libs/lib*.dylib 2>/dev/null` ; do
	
	EXPORTED_SYMBOLS="`nm "$so" | grep ' [BCDGINRSTVW] ' | grep -v ' _fini\>\| _init\>\| _fdata\>\| _ftext\>\| _fbss\>\| __bss_start\>\| __bss_start__\>\| __bss_end__\>\| _edata\>\| _end\>\| _bss_end__\>\| __end__\>' | cut -d' ' -f3`"
	prefix=`basename "$so" | sed 's/libharfbuzz/hb/; s/-/_/g; s/[.].*//'`

	echo "Processing $so"
	if echo "$EXPORTED_SYMBOLS" | grep -v "^${prefix}_"; then
		echo "Ouch, internal symbols exposed"
		stat=1
	fi

	tested=true
done
if ! $tested; then
	echo "check-symbols.sh: no shared library found; skipping test"
	exit 77
fi

exit $stat
