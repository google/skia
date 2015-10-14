TESTS += modules/objfmts/elf/tests/elf_test.sh

EXTRA_DIST += modules/objfmts/elf/tests/elf_test.sh
EXTRA_DIST += modules/objfmts/elf/tests/curpos.asm
EXTRA_DIST += modules/objfmts/elf/tests/curpos.hex
EXTRA_DIST += modules/objfmts/elf/tests/curpos-err.asm
EXTRA_DIST += modules/objfmts/elf/tests/curpos-err.errwarn
EXTRA_DIST += modules/objfmts/elf/tests/elf-overdef.asm
EXTRA_DIST += modules/objfmts/elf/tests/elf-overdef.hex
EXTRA_DIST += modules/objfmts/elf/tests/elf-x86id.asm
EXTRA_DIST += modules/objfmts/elf/tests/elf-x86id.hex
EXTRA_DIST += modules/objfmts/elf/tests/elfabssect.asm
EXTRA_DIST += modules/objfmts/elf/tests/elfabssect.hex
EXTRA_DIST += modules/objfmts/elf/tests/elfcond.asm
EXTRA_DIST += modules/objfmts/elf/tests/elfcond.hex
EXTRA_DIST += modules/objfmts/elf/tests/elfequabs.asm
EXTRA_DIST += modules/objfmts/elf/tests/elfequabs.hex
EXTRA_DIST += modules/objfmts/elf/tests/elfglobal.asm
EXTRA_DIST += modules/objfmts/elf/tests/elfglobal.hex
EXTRA_DIST += modules/objfmts/elf/tests/elfglobext.asm
EXTRA_DIST += modules/objfmts/elf/tests/elfglobext.hex
EXTRA_DIST += modules/objfmts/elf/tests/elfglobext2.asm
EXTRA_DIST += modules/objfmts/elf/tests/elfglobext2.hex
EXTRA_DIST += modules/objfmts/elf/tests/elfmanysym.asm
EXTRA_DIST += modules/objfmts/elf/tests/elfmanysym.hex
EXTRA_DIST += modules/objfmts/elf/tests/elfreloc.asm
EXTRA_DIST += modules/objfmts/elf/tests/elfreloc.hex
EXTRA_DIST += modules/objfmts/elf/tests/elfreloc-ext.asm
EXTRA_DIST += modules/objfmts/elf/tests/elfreloc-ext.hex
EXTRA_DIST += modules/objfmts/elf/tests/elfsectalign.asm
EXTRA_DIST += modules/objfmts/elf/tests/elfsectalign.hex
EXTRA_DIST += modules/objfmts/elf/tests/elfso.asm
EXTRA_DIST += modules/objfmts/elf/tests/elfso.hex
EXTRA_DIST += modules/objfmts/elf/tests/elftest.c
EXTRA_DIST += modules/objfmts/elf/tests/elftest.asm
EXTRA_DIST += modules/objfmts/elf/tests/elftest.hex
EXTRA_DIST += modules/objfmts/elf/tests/elftimes.asm
EXTRA_DIST += modules/objfmts/elf/tests/elftimes.hex
EXTRA_DIST += modules/objfmts/elf/tests/elftypesize.asm
EXTRA_DIST += modules/objfmts/elf/tests/elftypesize.hex
EXTRA_DIST += modules/objfmts/elf/tests/elfvisibility.asm
EXTRA_DIST += modules/objfmts/elf/tests/elfvisibility.errwarn
EXTRA_DIST += modules/objfmts/elf/tests/elfvisibility.hex
EXTRA_DIST += modules/objfmts/elf/tests/nasm-sectname.asm
EXTRA_DIST += modules/objfmts/elf/tests/nasm-sectname.hex
EXTRA_DIST += modules/objfmts/elf/tests/nasm-forceident.asm
EXTRA_DIST += modules/objfmts/elf/tests/nasm-forceident.hex

EXTRA_DIST += modules/objfmts/elf/tests/amd64/Makefile.inc
EXTRA_DIST += modules/objfmts/elf/tests/gas32/Makefile.inc
EXTRA_DIST += modules/objfmts/elf/tests/gas64/Makefile.inc

include modules/objfmts/elf/tests/amd64/Makefile.inc
include modules/objfmts/elf/tests/gas32/Makefile.inc
include modules/objfmts/elf/tests/gas64/Makefile.inc
