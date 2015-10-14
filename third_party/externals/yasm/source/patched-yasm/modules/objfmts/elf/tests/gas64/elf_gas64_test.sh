#! /bin/sh
${srcdir}/out_test.sh elf_gas64_test modules/objfmts/elf/tests/gas64 "GAS elf-amd64 objfmt" "-f elf64 -p gas" ".o"
exit $?
