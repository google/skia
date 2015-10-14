libyasm_a_SOURCES += modules/arch/x86/x86arch.c
libyasm_a_SOURCES += modules/arch/x86/x86arch.h
libyasm_a_SOURCES += modules/arch/x86/x86bc.c
libyasm_a_SOURCES += modules/arch/x86/x86expr.c
libyasm_a_SOURCES += modules/arch/x86/x86id.c
nodist_libyasm_a_SOURCES += x86cpu.c
nodist_libyasm_a_SOURCES += x86regtmod.c

YASM_MODULES += arch_x86

modules/arch/x86/x86id.c: x86insn_nasm.c x86insn_gas.c x86insns.c

EXTRA_DIST += modules/arch/x86/gen_x86_insn.py

if HAVE_PYTHON
x86insn_nasm.gperf x86insn_gas.gperf x86insns.c: $(srcdir)/modules/arch/x86/gen_x86_insn.py
	$(PYTHON) $(srcdir)/modules/arch/x86/gen_x86_insn.py
else
x86insn_nasm.gperf: $(srcdir)/x86insn_nasm.gperf
	@echo Python must be installed to regenerate x86 instructions files
	cp $(srcdir)/x86insn_nasm.gperf $@
x86insn_gas.gperf: $(srcdir)/x86insn_gas.gperf
	@echo Python must be installed to regenerate x86 instructions files
	cp $(srcdir)/x86insn_gas.gperf $@
endif

BUILT_SOURCES += x86insns.c
BUILT_SOURCES += x86insn_nasm.gperf
BUILT_SOURCES += x86insn_gas.gperf
EXTRA_DIST += x86insns.c
EXTRA_DIST += x86insn_nasm.gperf
EXTRA_DIST += x86insn_gas.gperf
MAINTAINERCLEANFILES += x86insns.c
MAINTAINERCLEANFILES += x86insn_nasm.gperf
MAINTAINERCLEANFILES += x86insn_gas.gperf

EXTRA_DIST += modules/arch/x86/x86cpu.gperf
EXTRA_DIST += modules/arch/x86/x86regtmod.gperf

# Use suffix rules for gperf files
x86insn_nasm.c: x86insn_nasm.gperf genperf$(EXEEXT)
x86insn_gas.c: x86insn_gas.gperf genperf$(EXEEXT)
x86cpu.c: $(srcdir)/modules/arch/x86/x86cpu.gperf genperf$(EXEEXT)
	$(top_builddir)/genperf$(EXEEXT) $(srcdir)/modules/arch/x86/x86cpu.gperf $@
x86regtmod.c: $(srcdir)/modules/arch/x86/x86regtmod.gperf genperf$(EXEEXT)
	$(top_builddir)/genperf$(EXEEXT) $(srcdir)/modules/arch/x86/x86regtmod.gperf $@

BUILT_SOURCES += x86insn_nasm.c
BUILT_SOURCES += x86insn_gas.c
CLEANFILES += x86insn_nasm.c
CLEANFILES += x86insn_gas.c
CLEANFILES += x86cpu.c
CLEANFILES += x86regtmod.c

EXTRA_DIST += modules/arch/x86/tests/Makefile.inc

include modules/arch/x86/tests/Makefile.inc
