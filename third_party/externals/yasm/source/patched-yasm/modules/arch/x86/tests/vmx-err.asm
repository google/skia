[bits 32]
;vmread [ebx], rcx
;vmwrite rbp, [ebp]

[bits 64]
vmread [rax], eax
vmwrite eax, [rcx]

