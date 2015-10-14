#! /bin/sh
${srcdir}/out_test.sh elf_amd64_test modules/objfmts/elf/tests/amd64 "elf-amd64 objfmt" "-m amd64 -f elf" ".o"
exit $?
