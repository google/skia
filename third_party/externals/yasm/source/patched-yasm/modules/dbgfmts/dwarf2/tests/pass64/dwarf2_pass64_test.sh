#! /bin/sh
${srcdir}/out_test.sh dwarf2_pass64_test modules/dbgfmts/dwarf2/tests/pass64 "dwarf2 dbgfmt pass64" "-f elf64 -p gas -g dwarf2" ".o"
exit $?
