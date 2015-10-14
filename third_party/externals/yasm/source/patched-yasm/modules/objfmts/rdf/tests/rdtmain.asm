	;; rdtmain - main part of test program for RDX execution.
	;; returns true (0) if its parameter equals the phrase "hello"
	;; "hello" is stored in the library part, to complicate the
	;; linkage.

	;; assemble and link with the following commands:
	;; nasm -f rdf rdtmain.asm
	;; nasm -f rdf rdtlib.asm
	;; ldrdf rdtmain.rdf rdtlib.rdf -o rdxtest.rdx

	;; run with 'rdx rdxtest.rdx [parameters]' on a Linux (or possibly
	;; other 32 bit OS) systems (x86 architectures only!)
	;; try using '&& echo Yes' afterwards to find out when it returns 0.
	
[EXTERN _strcmp]		; strcmp is an imported function
[EXTERN _message]		; imported data
[SECTION .text]
[BITS 32]

	;; main(int argc,char **argv)
[GLOBAL _main]
_main:
	push ebp
	mov ebp,esp

	;; ebp+8 = argc, ebp+12 = argv

	cmp dword [ebp+8],2
	jb error		; cause error if < 1 parameters

	mov eax, [ebp+12]	; eax = argv

	mov ebx, [eax+4]	; ebx = argv[1]
	mov ecx, _message	; ecx = "hello"

	push ecx
	push ebx
	call _strcmp		; compare strings
	add esp,8		; caller clears stack
	
	pop ebp
	ret			; return return value of _strcmp
	
error:
	mov eax,2		; return 2 on error
	pop ebp
	ret
