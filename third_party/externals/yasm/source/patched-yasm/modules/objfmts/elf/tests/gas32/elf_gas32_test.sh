#! /bin/sh
${srcdir}/out_test.sh elf_gas32_test modules/objfmts/elf/tests/gas32 "GAS elf-x86 objfmt" "-f elf32 -p gas" ".o"
exit $?
