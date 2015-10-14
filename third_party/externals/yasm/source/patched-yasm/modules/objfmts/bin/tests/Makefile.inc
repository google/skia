TESTS += modules/objfmts/bin/tests/bin_test.sh

EXTRA_DIST += modules/objfmts/bin/tests/bin_test.sh
EXTRA_DIST += modules/objfmts/bin/tests/abs.asm
EXTRA_DIST += modules/objfmts/bin/tests/abs.hex
EXTRA_DIST += modules/objfmts/bin/tests/bigorg.asm
EXTRA_DIST += modules/objfmts/bin/tests/bigorg.hex
EXTRA_DIST += modules/objfmts/bin/tests/bigorg.errwarn
EXTRA_DIST += modules/objfmts/bin/tests/bin-farabs.asm
EXTRA_DIST += modules/objfmts/bin/tests/bin-farabs.hex
EXTRA_DIST += modules/objfmts/bin/tests/bin-rip.asm
EXTRA_DIST += modules/objfmts/bin/tests/bin-rip.hex
EXTRA_DIST += modules/objfmts/bin/tests/bintest.asm
EXTRA_DIST += modules/objfmts/bin/tests/bintest.hex
EXTRA_DIST += modules/objfmts/bin/tests/float-err.asm
EXTRA_DIST += modules/objfmts/bin/tests/float-err.errwarn
EXTRA_DIST += modules/objfmts/bin/tests/float.asm
EXTRA_DIST += modules/objfmts/bin/tests/float.hex
EXTRA_DIST += modules/objfmts/bin/tests/integer-warn.asm
EXTRA_DIST += modules/objfmts/bin/tests/integer-warn.hex
EXTRA_DIST += modules/objfmts/bin/tests/integer-warn.errwarn
EXTRA_DIST += modules/objfmts/bin/tests/integer.asm
EXTRA_DIST += modules/objfmts/bin/tests/integer.hex
EXTRA_DIST += modules/objfmts/bin/tests/levelop.asm
EXTRA_DIST += modules/objfmts/bin/tests/levelop.hex
EXTRA_DIST += modules/objfmts/bin/tests/reserve.asm
EXTRA_DIST += modules/objfmts/bin/tests/reserve.hex
EXTRA_DIST += modules/objfmts/bin/tests/reserve.errwarn
EXTRA_DIST += modules/objfmts/bin/tests/shr.asm
EXTRA_DIST += modules/objfmts/bin/tests/shr.hex

EXTRA_DIST += modules/objfmts/bin/tests/multisect/Makefile.inc

include modules/objfmts/bin/tests/multisect/Makefile.inc
