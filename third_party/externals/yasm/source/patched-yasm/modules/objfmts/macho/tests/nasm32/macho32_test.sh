#! /bin/sh
${srcdir}/out_test.sh macho_test modules/objfmts/macho/tests/nasm32 "32-bit macho objfmt" "-f macho32" ".o"
exit $?
