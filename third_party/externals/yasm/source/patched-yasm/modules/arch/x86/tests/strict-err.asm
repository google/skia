bits 64
add [rax], dword 4		; illegal; must use dword [eax], 4
add [rax], strict dword 4	; illegal; must use dword [eax], strict dword 4
add [rax], qword 4		; illegal; must use qword [rax], 4
add [rax], strict qword 4	; illegal; must use qword [eax], strict dword 4
