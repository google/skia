#! /bin/sh
${srcdir}/out_test.sh gas_test modules/parsers/gas/tests "gas-compat parser" "-f elf -p gas" ".o"
exit $?
