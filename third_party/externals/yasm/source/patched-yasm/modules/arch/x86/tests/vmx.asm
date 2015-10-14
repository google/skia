[bits 32]
vmcall
vmclear [0]
vmlaunch
vmresume
vmptrld [0]
vmptrst [eax]
vmread [ebx], ecx
vmwrite ebp, [ebp]
vmxoff
vmxon [esi]

[bits 64]
vmcall
vmclear [0]
vmlaunch
vmresume
vmptrld [0]
vmptrst [rdx]
;vmread [rax], eax -- invalid
vmread [rax], rdx
vmwrite rax, [rcx]
vmxoff
vmxon [rsi]

