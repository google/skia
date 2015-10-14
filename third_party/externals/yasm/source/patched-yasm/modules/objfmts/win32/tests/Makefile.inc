TESTS += modules/objfmts/win32/tests/win32_test.sh

EXTRA_DIST += modules/objfmts/win32/tests/export.asm
EXTRA_DIST += modules/objfmts/win32/tests/export.hex
EXTRA_DIST += modules/objfmts/win32/tests/win32_test.sh
EXTRA_DIST += modules/objfmts/win32/tests/win32-curpos.asm
EXTRA_DIST += modules/objfmts/win32/tests/win32-curpos.hex
EXTRA_DIST += modules/objfmts/win32/tests/win32-overdef.asm
EXTRA_DIST += modules/objfmts/win32/tests/win32-overdef.hex
EXTRA_DIST += modules/objfmts/win32/tests/win32-safeseh.asm
EXTRA_DIST += modules/objfmts/win32/tests/win32-safeseh.hex
EXTRA_DIST += modules/objfmts/win32/tests/win32-safeseh.masm
EXTRA_DIST += modules/objfmts/win32/tests/win32-segof.asm
EXTRA_DIST += modules/objfmts/win32/tests/win32-segof.hex
EXTRA_DIST += modules/objfmts/win32/tests/win32test.c
EXTRA_DIST += modules/objfmts/win32/tests/win32test.asm
EXTRA_DIST += modules/objfmts/win32/tests/win32test.hex

EXTRA_DIST += modules/objfmts/win32/tests/gas/Makefile.inc

include modules/objfmts/win32/tests/gas/Makefile.inc
