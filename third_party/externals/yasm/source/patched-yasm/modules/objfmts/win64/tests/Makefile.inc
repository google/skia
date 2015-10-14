TESTS += modules/objfmts/win64/tests/win64_test.sh

EXTRA_DIST += modules/objfmts/win64/tests/win64_test.sh
EXTRA_DIST += modules/objfmts/win64/tests/sce1.asm
EXTRA_DIST += modules/objfmts/win64/tests/sce1.hex
EXTRA_DIST += modules/objfmts/win64/tests/sce1-err.asm
EXTRA_DIST += modules/objfmts/win64/tests/sce1-err.errwarn
EXTRA_DIST += modules/objfmts/win64/tests/sce2.asm
EXTRA_DIST += modules/objfmts/win64/tests/sce2.hex
EXTRA_DIST += modules/objfmts/win64/tests/sce2-err.asm
EXTRA_DIST += modules/objfmts/win64/tests/sce2-err.errwarn
EXTRA_DIST += modules/objfmts/win64/tests/sce3.asm
EXTRA_DIST += modules/objfmts/win64/tests/sce3.hex
EXTRA_DIST += modules/objfmts/win64/tests/sce3.masm
EXTRA_DIST += modules/objfmts/win64/tests/sce4.asm
EXTRA_DIST += modules/objfmts/win64/tests/sce4.hex
EXTRA_DIST += modules/objfmts/win64/tests/sce4.masm
EXTRA_DIST += modules/objfmts/win64/tests/sce4-err.asm
EXTRA_DIST += modules/objfmts/win64/tests/sce4-err.errwarn
EXTRA_DIST += modules/objfmts/win64/tests/win64-abs.asm
EXTRA_DIST += modules/objfmts/win64/tests/win64-abs.hex
EXTRA_DIST += modules/objfmts/win64/tests/win64-curpos.asm
EXTRA_DIST += modules/objfmts/win64/tests/win64-curpos.hex
EXTRA_DIST += modules/objfmts/win64/tests/win64-dataref.asm
EXTRA_DIST += modules/objfmts/win64/tests/win64-dataref.hex
EXTRA_DIST += modules/objfmts/win64/tests/win64-dataref.masm
EXTRA_DIST += modules/objfmts/win64/tests/win64-dataref2.asm
EXTRA_DIST += modules/objfmts/win64/tests/win64-dataref2.hex
EXTRA_DIST += modules/objfmts/win64/tests/win64-dataref2.masm
EXTRA_DIST += modules/objfmts/win64/tests/win64-imagebase.hex
EXTRA_DIST += modules/objfmts/win64/tests/win64-imagebase.asm

EXTRA_DIST += modules/objfmts/win64/tests/gas/Makefile.inc

include modules/objfmts/win64/tests/gas/Makefile.inc
