libyasm_a_SOURCES += modules/objfmts/elf/elf.c
libyasm_a_SOURCES += modules/objfmts/elf/elf.h
libyasm_a_SOURCES += modules/objfmts/elf/elf-objfmt.c
libyasm_a_SOURCES += modules/objfmts/elf/elf-machine.h
libyasm_a_SOURCES += modules/objfmts/elf/elf-x86-x86.c
libyasm_a_SOURCES += modules/objfmts/elf/elf-x86-amd64.c

YASM_MODULES += objfmt_elf objfmt_elf32 objfmt_elf64

EXTRA_DIST += modules/objfmts/elf/tests/Makefile.inc

include modules/objfmts/elf/tests/Makefile.inc
