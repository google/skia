#! /bin/sh
${srcdir}/out_test.sh x86_gas32_test modules/arch/x86/tests/gas32 "x86 gas format" "-f elf32 -p gas" ".o"
exit $?
