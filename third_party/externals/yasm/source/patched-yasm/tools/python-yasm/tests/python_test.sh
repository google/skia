#!/bin/sh
# Based on _sanity.sh from Quod Libet
# http://www.sacredchao.net/quodlibet/

set -e

test -n "${srcdir}" || srcdir=.
test -n "${PYTHON}" || PYTHON=python

if test "$1" = "--help" -o "$1" = "-h"; then
    echo "Usage: $0 --sanity | [TestName] ..."
    exit 0
elif [ "$1" = "--sanity" ]; then
    echo "Running static sanity checks."
    grep "except None:" ${srcdir}/tools/python-yasm/tests/*.py
else
    ${PYTHON} -c "import sys; import glob; sys.path.insert(0, '${srcdir}/tools/python-yasm'); sys.path.insert(0, glob.glob('build/lib.*')[0]); import tests; raise SystemExit(tests.unit('$*'.split()))"
fi

