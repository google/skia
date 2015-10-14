
call    _foo                
# r_type= X86_64_RELOC_BRANCH, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
# E8 00 00 00 00 

call    _foo+4                
# r_type=X86_64_RELOC_BRANCH, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
# E8 04 00 00 00 

# TODO: movq _foo@GOTPCREL(%rip), %rax
# r_type=X86_64_RELOC_GOT_LOAD, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
# 48 8B 05 00 00 00 00

# TODO: pushq _foo@GOTPCREL(%rip)
# r_type=X86_64_RELOC_GOT, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
# FF 35 00 00 00 00

movl _foo(%rip), %eax
# r_type=X86_64_RELOC_SIGNED, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
# 8B 05 00 00 00 00 

movl _foo+4(%rip), %eax
# r_type= X86_64_RELOC_SIGNED, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
# 8B 05 04 00 00 00 

movb  $0x12, _foo(%rip)
# r_type= X86_64_RELOC_SIGNED, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
# C6 05 FF FF FF FF 12 

movl  $0x12345678, _foo(%rip)
# r_type= X86_64_RELOC_SIGNED, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
# C7 05 FC FF FF FF 78 56 34 12

.quad _foo
# r_type=X86_64_RELOC_UNSIGNED,r_length=3, r_extern=1,r_pcrel=0, r_symbolnum=_foo
# 00 00 00 00 00 00 00 00

.quad _foo+4
# r_type=X86_64_RELOC_UNSIGNED,r_length=3,r_extern=1,r_pcrel=0,r_symbolnum=_foo
# 04 00 00 00 00 00 00 00

# TODO: .quad _foo - _bar
# r_type=X86_64_RELOC_SUBTRACTOR,r_length=3,r_extern=1, r_pcrel=0,r_symbolnum=_bar
# r_type=X86_64_RELOC_UNSIGNED,r_length=3,r_extern=1, r_pcrel=0,r_symbolnum=_foo
# 00 00 00 00 00 00 00 00

# TODO: .quad _foo - _bar + 4
# r_type=X86_64_RELOC_SUBTRACTOR,r_length=3, r_extern=1,r_pcrel=0,r_symbolnum=_bar
# r_type=X86_64_RELOC_UNSIGNED,r_length=3, r_extern=1,r_pcrel=0,r_symbolnum=_foo
# 04 00 00 00 00 00 00 00

# TODO: .long _foo - _bar
# r_type=X86_64_RELOC_SUBTRACTOR,r_length=2,r_extern=1,r_pcrel=0,r_symbolnum=_bar
# r_type=X86_64_RELOC_UNSIGNED,r_length=2,r_extern=1,r_pcrel=0,r_symbolnum=_foo
# 00 00 00 00 

lea L1(%rip), %rax
# r_type=X86_64_RELOC_SIGNED, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_prev
# 48 8d 05 12 00 00 00 
# Assumes that _prev is the first nonlocal label 0x12 bytes before L1.
 
lea L0(%rip), %rax
# r_type= X86_64_RELOC_SIGNED, r_length=2, r_extern=0, r_pcrel=1, r_symbolnum=3
# 48 8d 05 56 00 00 00 
# Assumes that  L0 is in third section, and has an address of 0x00000056 
# in .o file, and no previous nonlocal label.
 
.quad L1
# r_type=X86_64_RELOC_UNSIGNED,r_length=3,r_extern=1,r_pcrel=0, r_symbolnum= _prev
# 12 00 00 00 00 00 00 00
# Assumes that _prev is the first nonlocal label 0x12 bytes before L1.
 
.quad L0
# r_type=X86_64_RELOC_UNSIGNED,r_length=3, r_extern=0, r_pcrel=0, r_symbolnum= 3
# 56 00 00 00 00 00 00 00
# Assumes that L0 is in third section, and has address of 0x00000056 
# in .o file, and no previous nonlocal label.
 
# TODO: .quad _foo - .
# r_type=X86_64_RELOC_SUBTRACTOR,r_length=3,r_extern=1,r_pcrel=0,r_symbolnum=_prev
# r_type=X86_64_RELOC_UNSIGNED,r_length=3,r_extern=1,r_pcrel=0,r_symbolnum=_foo
# EE FF FF FF FF FF FF FF
# Assumes that _prev is the first nonlocal label 0x12 bytes 
# before this .quad 
 
# TODO: .quad _foo - L1
# r_type=X86_64_RELOC_SUBTRACTOR,r_length=3,r_extern=1,r_pcrel=0,r_symbolnum=_prev
# r_type=X86_64_RELOC_UNSIGNED,r_length=3,r_extern=1,r_pcrel=0,r_symbolnum=_foo
# EE FF FF FF FF FF FF FF
# Assumes that  _prev is the first nonlocal label 0x12 bytes before L1. 
 
.quad L1 - _prev
# No relocations. This is an assembly time constant.
# 12 00 00 00 00 00 00 00
# Assumes that _prev is the first nonlocal label 0x12 bytes before L

.data
.org 0x56
L0:
_prev:
.quad 0, 0
.byte 0, 0
L1:
