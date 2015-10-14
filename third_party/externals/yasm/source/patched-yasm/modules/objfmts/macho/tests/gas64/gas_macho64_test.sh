#! /bin/sh
${srcdir}/out_test.sh macho_test modules/objfmts/macho/tests/gas64 "GAS 64-bit macho objfmt" "-f macho64 -p gas" ".o"
exit $?
