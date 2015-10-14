#! /bin/sh
${srcdir}/out_test.sh x86_gas64_test modules/arch/x86/tests/gas64 "amd64 gas format" "-f elf -m amd64 -p gas" ".o"
exit $?
