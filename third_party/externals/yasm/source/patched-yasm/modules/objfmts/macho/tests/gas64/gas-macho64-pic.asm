call    _foo
# r_type=X86_64_RELOC_BRANCH, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
# E8 00 00 00 00

call    _foo+4
# r_type=X86_64_RELOC_BRANCH, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
# E8 04 00 00 00

movq _foo@GOTPCREL(%rip), %rax
# r_type=X86_64_RELOC_GOT_LOAD, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
# 48 8B 05 00 00 00 00

pushq _foo@GOTPCREL(%rip)
# r_type=X86_64_RELOC_GOT, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
# FF 35 00 00 00 00

movl _foo(%rip), %eax
# r_type=X86_64_RELOC_SIGNED, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
# 8B 05 00 00 00 00

movl _foo+4(%rip), %eax
# r_type=X86_64_RELOC_SIGNED, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
# 8B 05 04 00 00 00

movb  $0x12, _foo(%rip)
# r_type=X86_64_RELOC_SIGNED, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
# C6 05 FF FF FF FF 12

movl  $0x12345678, _foo(%rip)
# r_type=X86_64_RELOC_SIGNED, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
# C7 05 FC FF FF FF 78 56 34 12

