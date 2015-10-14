[bits 64]
mov ah, 5
mov ax, 5
mov eax, 5
mov rax, 5		; optimized to signed 32-bit form
mov rax, dword 5	; explicitly 32-bit
mov rax, qword 5	; explicitly 64-bit
; test sign optimization cases
mov rax, 0x7fffffff
mov rax, dword 0x7fffffff
mov rax, qword 0x7fffffff
mov rax, 0x80000000
mov rax, dword 0x80000000
mov rax, qword 0x80000000
mov rax, -0x80000000
mov rax, dword -0x80000000
mov rax, qword -0x80000000
mov rax, 0x100000000
mov rax, dword 0x100000000
mov rax, qword 0x100000000
mov ah, bl
mov bl, r8b
mov sil, r9b
mov r10w, r11w
mov r15d, r12d
mov r13, r14
inc ebx
dec ecx
