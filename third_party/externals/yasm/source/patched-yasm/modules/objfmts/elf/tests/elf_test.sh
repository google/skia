#! /bin/sh
# copied from yasm/modules/objfmts/coff/tests/coff_test.sh ; s/coff/elf/g
${srcdir}/out_test.sh elf_test modules/objfmts/elf/tests "elf objfmt" "-f elf" ".o"
exit $?
