; test source file for assembling to MACH-O 
; build with :
;    yasm -f macho -m amd64 machotest64.asm
;    gcc -m64 -o machotest64 machotest64.c machotest64.o

; This file should test the following:
; [1] Define and export a global text-section symbol
; [2] Define and export a global data-section symbol
; [3] Define and export a global BSS-section symbol
; [4] Define a non-global text-section symbol
; [5] Define a non-global data-section symbol
; [6] Define a non-global BSS-section symbol
; [7] Define a COMMON symbol
; [8] Define a NASM local label
; [9] Reference a NASM local label
; [10] Import an external symbol (note: printf replaced by another call)
; [11] Make a PC-relative call to an external symbol
; [12] Reference a text-section symbol in the text section
; [13] Reference a data-section symbol in the text section
; [14] Reference a BSS-section symbol in the text section
; [15] Reference a text-section symbol in the data section
; [16] Reference a data-section symbol in the data section
; [17] Reference a BSS-section symbol in the data section
; [18] Perform a 64 Bit relocation in the text section

[BITS 64]
[GLOBAL _lrotate]	; [1]
[GLOBAL _greet]		; [1]
[GLOBAL _asmstr]	; [2]
[GLOBAL _textptr]	; [2]
[GLOBAL _selfptr]	; [2]
[GLOBAL _integer]	; [3]
[EXTERN _druck]		; [10]
[COMMON _commvar 4]	; [7]
[GLOBAL _getstr]	;
[GLOBAL _readgreet]	;

[SECTION .text]

; prototype: long lrotate(long x, int num);
_lrotate:			; [1]
	push	rcx
	mov	rax,rdi
	mov	rcx,rsi
.label	  rol rax,1		; [4] [8]
	  loop .label		; [9] [12]
	  pop rcx
	  ret

_getstr:
	mov	rax,qword _asmstr
	ret

_readgreet:
	mov rax,[qword localint]	; [18]
	ret

_retrievelabel:
	mov rax,[qword localptr]
	ret

; prototype: void greet(void);
; calls "void druck(a,b,c,d);
_greet	  mov rax,[_integer wrt rip]	; [14]
	  inc rax
	  mov [localint wrt rip],rax	; [14]
	push rdi
	push rsi
	push rdx
	push rcx
	mov rdi,qword _printfstr
	mov rsi,[_integer wrt rip]
	mov rdx,[localptr wrt rip]
	mov rdx,[rdx]
	mov rcx,[_commvar wrt rip]
        call _druck
	pop rcx
	pop rdx
	pop rsi
	pop rdi
	  ret

; some internal calls
	call _greet
	call _retrievelabel

[SECTION .data]

; a string for Printf 
_printfstr db "integer==%d, localint==%d, commvar=%d"
	  db 10, 0

; some pointers
localptr  dq localint		; [5] [17]
_textptr  dq _greet		; [15]
_selfptr  dq _selfptr		; [16]

;[section .data2 align=16]

; a string
_asmstr	  db 'hello, world', 0	; [2]



[SECTION .bss]

; an integer
_integer  resq 1		; [3]

; a local integer
localint  resq 1		; [6]
