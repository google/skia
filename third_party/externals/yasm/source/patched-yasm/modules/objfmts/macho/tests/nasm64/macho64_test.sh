#! /bin/sh
${srcdir}/out_test.sh macho_test modules/objfmts/macho/tests/nasm64 "64-bit macho objfmt" "-f macho64" ".o"
exit $?
