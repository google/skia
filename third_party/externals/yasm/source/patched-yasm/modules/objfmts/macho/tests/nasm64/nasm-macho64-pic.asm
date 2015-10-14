[extern _foo]

call    _foo
; r_type=X86_64_RELOC_BRANCH, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
; E8 00 00 00 00

call    _foo+4
; r_type=X86_64_RELOC_BRANCH, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
; E8 04 00 00 00

mov rax, [rel _foo wrt ..gotpcrel]
; r_type=X86_64_RELOC_GOT_LOAD, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
; 48 8B 05 00 00 00 00

push qword [rel _foo wrt ..gotpcrel]
; r_type=X86_64_RELOC_GOT, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
; FF 35 00 00 00 00

mov eax, [rel _foo]
; r_type=X86_64_RELOC_SIGNED, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
; 8B 05 00 00 00 00

mov eax, [rel _foo+4]
; r_type=X86_64_RELOC_SIGNED, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
; 8B 05 04 00 00 00

mov [rel _foo], byte 12h
; r_type=X86_64_RELOC_SIGNED, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
; C6 05 FF FF FF FF 12

mov dword [rel _foo], 0x12345678
; r_type=X86_64_RELOC_SIGNED, r_length=2, r_extern=1, r_pcrel=1, r_symbolnum=_foo
; C7 05 FC FF FF FF 78 56 34 12

