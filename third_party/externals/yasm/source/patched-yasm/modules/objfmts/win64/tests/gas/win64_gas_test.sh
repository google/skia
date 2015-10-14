#! /bin/sh
${srcdir}/out_test.sh win64_gas_test modules/objfmts/win64/tests/gas "win64 objfmt" "-f win64 -p gas -r nasm" ".obj"
exit $?
