	;; program to test RDOFF production and linkage

	;; items to test include:
	;;	[1] relocation within the same segment in each module
	;;	[2] relocation to different segments in same module
	;;	[3] relocation to same segment in different module
	;;	[4] relocation to different segment in different module
	;;	[5] relative relocation to same module
	;;	[6] relative relocation to different module
	;;	[7] correct generation of BSS addresses

[SECTION .text]
[BITS 32]
	
_main:
	mov ax,localdata	; [2] (16 bit) => 66 b8 0000
	mov eax,localdata2	; [2] (32 bit) => b8 0000000a

[EXTERN _fardata]

	mov eax,[_fardata]	; [4] => a1 00000000 (+20)
	mov cx,next		; [1] => 66 b9 0012
next:
	call localproc		; [5] => e8 00000019

[EXTERN _farproc]
	mov eax,_farproc	; [3] => b8 00000000 (+40+0)
	call _farproc		; [6] => e8 -$ (-0+40+0) (=1f)

	mov eax,localbss	; [7] => b8 00000000

[GLOBAL _term]
_term:	xor ax,ax		; => 66 31 c0
	int 21h			; => cd 21
	jmp _term		; => e9 -0a (=fffffff6)

localproc:	
	ret			; => c3

[GLOBAL _test1proc]
_test1proc:
	call localproc		; [5] => e8 -$ (-0+0+?) (=-6=fffffffa)
	ret			; => c3
			
[SECTION .data]
[GLOBAL localdata2]
localdata:	db 'localdata',0
localdata2:	db 'localdata2',0
farref:		dd _fardata	; [3] => 0 (+20)
localref:	dd _main	; [2] => 0 (+0)

[SECTION .bss]
localbss:	resw 4		; reserve 8 bytes BSS
	