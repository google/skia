	;; library functions for rdtmain - test of rdx linking and execution

	;; library function = _strcmp, defined as in C

[SECTION .text]
[BITS 32]

[GLOBAL _strcmp]
_strcmp:
	push ebp
	mov ebp,esp

	;; ebp+8 = first paramater, ebp+12 = second

	mov esi,[ebp+8]
	mov edi,[ebp+12]

.loop:
	mov cl,byte [esi]
	mov dl,byte [edi]
	cmp cl,dl
	jb .below
	ja .above
	or cl,cl
	jz .match
	inc esi
	inc edi
	jmp .loop

.below:	
	mov eax,-1
	pop ebp
	ret
	
.above:
	mov eax,1
	pop ebp
	ret

.match:
	xor eax,eax
	pop ebp
	ret

[SECTION .data]
[GLOBAL _message]

_message:	db 'hello',0