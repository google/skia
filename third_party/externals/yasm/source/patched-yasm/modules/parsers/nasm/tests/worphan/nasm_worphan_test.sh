#! /bin/sh
${srcdir}/out_test.sh nasm_test modules/parsers/nasm/tests/worphan "nasm-compat parser" "-Worphan-labels -f bin" ""
exit $?
