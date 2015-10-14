#! /bin/sh
# copied from yasm/modules/objfmts/coff/tests/coff_test.sh ; s/coff/stabs/g
${srcdir}/out_test.sh stabs_test modules/dbgfmts/stabs/tests "stabs dbgfmt" "-f elf -g stabs" ".o"
exit $?
