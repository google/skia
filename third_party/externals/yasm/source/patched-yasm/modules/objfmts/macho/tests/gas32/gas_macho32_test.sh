#! /bin/sh
${srcdir}/out_test.sh macho_test modules/objfmts/macho/tests/gas32 "GAS 32-bit macho objfmt" "-f macho32 -p gas" ".o"
exit $?
