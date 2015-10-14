	;; rdftest2.asm - test linkage and generation of RDOFF files

[SECTION .text]
[BITS 32]

[GLOBAL _farproc]
[EXTERN _test1proc]
[EXTERN localdata2]
[EXTERN _term]
_farproc:
	
	mov bx,localdata2	; [4] 0 => 66 bb 000a(+0)
	mov eax,_term		; [3] 5 => b8 00000000(+26+0)
	call _test1proc		; [6] A => e8 fffffff2(-40+0+31)(=ffffffe3)

	mov eax,_farproc	; [1] => b8 00000000(+40)
	add eax,[_fardata]	; [2] => 03 05 00000000(+20)

	mov ebx,mybssdata	; [7] => bb 00000000(+08)
	call myproc		; [5] => e8 00000001
	ret

myproc:
	add eax,ebx
	ret
	
[SECTION .data]
[GLOBAL _fardata]
_fardata:	dw _term	; [4]
_localref:	dd _farproc	; [2]

[SECTION .bss]
mybssdata:	resw 1
