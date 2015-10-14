EXTRA_DIST += modules/preprocs/tasm/Makefile.inc
EXTRA_DIST += modules/preprocs/nasm/Makefile.inc
EXTRA_DIST += modules/preprocs/raw/Makefile.inc
EXTRA_DIST += modules/preprocs/cpp/Makefile.inc
EXTRA_DIST += modules/preprocs/gas/Makefile.inc

include modules/preprocs/tasm/Makefile.inc
include modules/preprocs/nasm/Makefile.inc
include modules/preprocs/raw/Makefile.inc
include modules/preprocs/cpp/Makefile.inc
include modules/preprocs/gas/Makefile.inc
