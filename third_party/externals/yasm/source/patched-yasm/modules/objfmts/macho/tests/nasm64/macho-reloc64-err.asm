[BITS 64]
[SECTION .data]

uhoh db 5

[GLOBAL blah]

blah dw 5
[GLOBAL aha]
aha	dq	blah
aha2	dq	blah+4
aha3	dq	blah-uhoh

[SECTION .text]

[EXTERN hi]
[EXTERN hi]
[EXTERN bye]
[BITS 64]
	mov rax, hi+2
	mov rax, bye
	mov rax, [qword hi]
	mov rdi, [rip+ hi]
	mov rax, [bye+2]
	mov rax, $$
	mov rax, $
	mov rax, $+4
	mov rax, $-$$
mov eax, uhoh wrt $$
;mov eax, hi+bye
;mov eax, bye+$
;mov eax, hi-$
