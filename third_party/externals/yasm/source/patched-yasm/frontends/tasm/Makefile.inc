bin_PROGRAMS += ytasm

ytasm_SOURCES  = frontends/tasm/tasm.c
ytasm_SOURCES += frontends/tasm/tasm-options.c
ytasm_SOURCES += frontends/tasm/tasm-options.h

ytasm_LDADD = libyasm.a $(INTLLIBS)
