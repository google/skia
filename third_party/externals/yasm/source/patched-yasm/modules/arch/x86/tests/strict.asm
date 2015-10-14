bits 32

;jmp strict near foo
;jmp near foo
;jmp strict short foo
;jmp short foo
;jmp foo
;
;jz strict near foo
;jz near foo
;jz strict short foo
;jz short foo
;jz foo
;
;foo:

add eax, 4
add eax, strict 4		; NASM generates dword, yasm generates byte
add eax, byte 4
add eax, strict byte 4
add eax, dword 4		; optimized to byte
add eax, strict dword 4
add eax, 400
add eax, strict 400
add eax, byte 400		; generates warning
add eax, strict byte 400	; generates warning
add eax, dword 400		; optimized to byte
add eax, strict dword 400

add ebx, 4
add ebx, strict 4		; NASM generates dword, yasm generates byte
add ebx, byte 4
add ebx, strict byte 4
add ebx, dword 4		; optimized to byte
add ebx, strict dword 4
add ebx, 400
add ebx, strict 400
add ebx, byte 400		; generates warning
add ebx, strict byte 400	; generates warning
add ebx, dword 400		; optimized to byte
add ebx, strict dword 400

add [eax], byte 4		; same as byte [eax], 4
add [eax], strict byte 4	; same as byte [eax], 4
add [eax], dword 4		; generates dword [eax], byte 4
add [eax], strict dword 4	; generates dword [eax], dword 4

add dword [eax], 4
add dword [eax], strict 4	; NASM generates dword, yasm generates byte
add dword [eax], byte 4
add dword [eax], strict byte 4
add dword [eax], dword 4	; optimized to byte
add dword [eax], strict dword 4
add dword [eax], 400
add dword [eax], strict 400
add dword [eax], byte 400	; generates warning
add dword [eax], strict byte 400; generates warning
add dword [eax], dword 400	; optimized to byte
add dword [eax], strict dword 400

push 4
push strict 4			; NASM generates dword, yasm generates byte
push byte 4
push strict byte 4
push dword 4			; optimized to byte
push strict dword 4
push 400
push strict 400
push byte 400			; generates warning
push strict byte 400		; generates warning
push dword 400
push strict dword 400

imul eax, 4
imul eax, strict 4		; NASM generates dword, yasm generates byte
imul eax, byte 4
imul eax, strict byte 4
imul eax, dword 4		; optimized to byte
imul eax, strict dword 4
imul eax, 400
imul eax, strict 400
imul eax, byte 400		; generates warning
imul eax, strict byte 400	; generates warning
imul eax, dword 400
imul eax, strict dword 400

%ifndef __NASM_VERSION_ID__
bits 64
add rax, 4
add rax, strict 4		; NASM generates dword, yasm generates byte
add rax, byte 4
add rax, strict byte 4
add rax, dword 4
add rax, strict dword 4
add rax, 400
add rax, strict 400
add rax, byte 400		; generates warning
add rax, strict byte 400	; generates warning
add rax, dword 400
add rax, strict dword 400

add rbx, 4
add rbx, strict 4		; NASM generates dword, yasm generates byte
add rbx, byte 4
add rbx, strict byte 4
add rbx, dword 4
add rbx, strict dword 4
add rbx, 400
add rbx, strict 400
add rbx, byte 400		; generates warning
add rbx, strict byte 400	; generates warning
add rbx, dword 400
add rbx, strict dword 400

add [rax], byte 4		; same as byte [rax], 4
add [rax], strict byte 4	; same as byte [rax], 4
add [rax], word 4		; same as word [rax], 4
add [rax], strict word 4	; same as word [rax], strict word 4

add dword [rax], 4
add dword [rax], strict 4
add dword [rax], byte 4
add dword [rax], strict byte 4
add dword [rax], dword 4
add dword [rax], strict dword 4
add dword [rax], 400
add dword [rax], strict 400
add dword [rax], byte 400	; generates warning
add dword [rax], strict byte 400; generates warning
add dword [rax], dword 400
add dword [rax], strict dword 400

add qword [rax], 4
add qword [rax], strict 4
add qword [rax], byte 4
add qword [rax], strict byte 4
add qword [rax], dword 4
add qword [rax], strict dword 4
add qword [rax], 400
add qword [rax], strict 400
add qword [rax], byte 400	; generates warning
add qword [rax], strict byte 400; generates warning
add qword [rax], dword 400
add qword [rax], strict dword 400

push 4
push strict 4			; NASM generates dword, yasm generates byte
push byte 4
push strict byte 4
push dword 4			; optimized to byte
push strict dword 4
;push qword 4			; illegal
;push strict qword 4		; illegal
push 400
push strict 400
push byte 400			; generates warning
push strict byte 400		; generates warning
push dword 400
push strict dword 400
;push qword 400			; illegal
;push strict qword 400		; illegal

%endif
