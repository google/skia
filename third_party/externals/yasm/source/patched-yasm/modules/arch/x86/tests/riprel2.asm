	bits 64

	default abs	; default abs, except for explicit rel

	mov rax,[foo]
	mov rax,[qword 123456789abcdef0h]
	mov rbx,[foo]
	mov rax,[dword foo]
	mov rbx,[dword foo]
	mov rax,[qword foo]
	mov rax,[rel foo]		; rel
	mov rbx,[rel foo]		; rel
	mov rax,[rel dword foo]		; rel
	;mov rax,[rel qword foo]	; illegal
	mov rax,[abs foo]
	mov rbx,[abs foo]
	mov rax,[abs dword foo]
	mov rax,[abs qword foo]

	mov rax,[es:foo]
	mov rax,[qword es:123456789abcdef0h]
	mov rbx,[es:foo]
	mov rax,[dword es:foo]
	mov rbx,[dword es:foo]
	mov rax,[qword es:foo]
	mov rax,[rel es:foo]		; rel
	mov rbx,[rel es:foo]		; rel
	mov rax,[rel dword es:foo]	; rel
	;mov rax,[rel qword es:foo]	; illegal
	mov rax,[abs es:foo]
	mov rbx,[abs es:foo]
	mov rax,[abs dword es:foo]
	mov rax,[abs qword es:foo]

	mov rax,[fs:foo]
	mov rax,[qword fs:123456789abcdef0h]
	mov rbx,[fs:foo]
	mov rax,[dword fs:foo]
	mov rbx,[dword fs:foo]
	mov rax,[qword fs:foo]
	mov rax,[rel fs:foo]		; rel
	mov rbx,[rel fs:foo]		; rel
	mov rax,[rel dword fs:foo]	; rel
	;mov rax,[rel qword fs:foo]	; illegal
	mov rax,[abs fs:foo]
	mov rbx,[abs fs:foo]
	mov rax,[abs dword fs:foo]
	mov rax,[abs qword fs:foo]

	mov rax,[rbx]
	mov rax,[rel rbx]
	mov rax,[abs rbx]

	default rel

	; all of these are default rel, except for 64-bit displacements
	mov rax,[foo]
	mov rax,[qword 123456789abcdef0h]	; abs
	mov rbx,[foo]
	mov rax,[dword foo]
	mov rbx,[dword foo]
	mov rax,[qword foo]		; abs
	mov rax,[rel foo]
	mov rbx,[rel foo]
	mov rax,[rel dword foo]
	;mov rax,[rel qword foo]	; illegal
	mov rax,[abs foo]
	mov rbx,[abs foo]
	mov rax,[abs dword foo]
	mov rax,[abs qword foo]

	; all of these are default rel, except for 64-bit displacements
	mov rax,[es:foo]
	mov rax,[qword es:123456789abcdef0h]
	mov rbx,[es:foo]
	mov rax,[dword es:foo]
	mov rbx,[dword es:foo]
	mov rax,[qword es:foo]
	mov rax,[rel es:foo]		; rel
	mov rbx,[rel es:foo]		; rel
	mov rax,[rel dword es:foo]	; rel
	;mov rax,[rel qword es:foo]	; illegal
	mov rax,[abs es:foo]
	mov rbx,[abs es:foo]
	mov rax,[abs dword es:foo]
	mov rax,[abs qword es:foo]

	; all of these are abs due to fs:, except for explicit rel
	mov rax,[fs:foo]
	mov rax,[qword fs:123456789abcdef0h]
	mov rbx,[fs:foo]
	mov rax,[dword fs:foo]
	mov rbx,[dword fs:foo]
	mov rax,[qword fs:foo]
	mov rax,[rel fs:foo]		; rel
	mov rbx,[rel fs:foo]		; rel
	mov rax,[rel dword fs:foo]	; rel
	;mov rax,[rel qword fs:foo]	; illegal
	mov rax,[abs fs:foo]
	mov rbx,[abs fs:foo]
	mov rax,[abs dword fs:foo]
	mov rax,[abs qword fs:foo]

	mov rax,[rbx]
	mov rax,[rel rbx]
	mov rax,[abs rbx]

	section .data
foo	equ $
	
