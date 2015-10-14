bits 64
val:
default abs

mov rax, val			; 32-bit imm
mov rax, dword val		; 32-bit imm
mov rax, qword val		; 64-bit imm

mov rbx, val			; 32-bit imm
mov rbx, dword val		; 32-bit imm
mov rbx, qword val		; 64-bit imm

mov rax, [val]			; 48 8b ... (32-bit disp)
mov rax, [dword val]		; 48 8b ... (32-bit disp)
mov rax, [qword val]		; 48 a1 ... (64-bit disp)
a32 mov rax, [val]		; 67 48 a1 ... (32-bit disp)
a32 mov rax, [dword val]	; 67 48 a1 ... (32-bit disp)
a32 mov rax, [qword val]	; 67 48 a1 ... (32-bit disp)
				; [this one is debatable on correctness,
				; I chose in yasm to make a32 override]
a64 mov rax, [val]		; 48 8b ... (32-bit disp)
a64 mov rax, [dword val]	; 48 8b ... (32-bit disp)
a64 mov rax, [qword val]	; 48 a1 ... (64-bit disp)

mov rbx, [val]			; 48 8b ... (32-bit disp)
mov rbx, [dword val]		; 48 8b ... (32-bit disp)
;mov rbx, [qword val]		; illegal (can't have 64-bit disp)
a32 mov rbx, [val]		; 67 48 8b ... (32-bit disp)
a32 mov rbx, [dword val]	; 67 48 8b ... (32-bit disp)
;a32 mov rbx, [qword val]	; illegal (can't have 64-bit disp)
a64 mov rbx, [val]		; 48 8b ... (32-bit disp)
a64 mov rbx, [dword val]	; 48 8b ... (32-bit disp)
;a64 mov rbx, [qword val]	; illegal (can't have 64-bit disp)

default rel

mov rax, val			; 32-bit imm
mov rax, dword val		; 32-bit imm
mov rax, qword val		; 64-bit imm

mov rbx, val			; 32-bit imm
mov rbx, dword val		; 32-bit imm
mov rbx, qword val		; 64-bit imm

mov rax, [val]			; 48 8b ... (32-bit disp, RIP-rel)
mov rax, [dword val]		; 48 8b ... (32-bit disp, RIP-rel)
mov rax, [qword val]		; 48 a1 ... (64-bit disp, ABS)
a32 mov rax, [val]		; 67 48 8b ... (32-bit disp, RIP-rel)
a32 mov rax, [dword val]	; 67 48 8b ... (32-bit disp, RIP-rel)
a32 mov rax, [qword val]	; 67 48 a1 ... (32-bit disp, ABS)
				; [this one is debatable on correctness,
				; I chose in yasm to make a32 override]
a64 mov rax, [val]		; 48 8b ... (32-bit disp, RIP-rel)
a64 mov rax, [dword val]	; 48 8b ... (32-bit disp, RIP-rel)
a64 mov rax, [qword val]	; 48 a1 ... (64-bit disp, ABS)

mov rbx, [val]			; 48 8b ... (32-bit disp, RIP-rel)
mov rbx, [dword val]		; 48 8b ... (32-bit disp, RIP-rel)
;mov rbx, [qword val]		; illegal (can't have 64-bit disp)
a32 mov rbx, [val]		; 67 48 8b ... (32-bit disp, RIP-rel)
a32 mov rbx, [dword val]	; 67 48 8b ... (32-bit disp, RIP-rel)
;a32 mov rbx, [qword val]	; illegal (can't have 64-bit disp)
a64 mov rbx, [val]		; 48 8b ... (32-bit disp, RIP-rel)
a64 mov rbx, [dword val]	; 48 8b ... (32-bit disp, RIP-rel)
;a64 mov rbx, [qword val]	; illegal (can't have 64-bit disp)

