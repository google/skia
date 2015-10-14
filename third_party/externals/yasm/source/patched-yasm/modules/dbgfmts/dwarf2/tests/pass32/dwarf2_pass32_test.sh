#! /bin/sh
${srcdir}/out_test.sh dwarf2_pass32_test modules/dbgfmts/dwarf2/tests/pass32 "dwarf2 dbgfmt pass32" "-f elf -p gas -g dwarf2" ".o"
exit $?
