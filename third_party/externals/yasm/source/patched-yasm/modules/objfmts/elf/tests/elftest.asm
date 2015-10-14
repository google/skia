; test source file for assembling to ELF
; copied from cofftest.asm;  s/_//g  s/coff/elf/g
; build with (under Linux, for example):
;    yasm -f elf elftest.asm
;    gcc -o elftest elftest.c elftest.o

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
; [10] Import an external symbol
; [11] Make a PC-relative call to an external symbol
; [12] Reference a text-section symbol in the text section
; [13] Reference a data-section symbol in the text section
; [14] Reference a BSS-section symbol in the text section
; [15] Reference a text-section symbol in the data section
; [16] Reference a data-section symbol in the data section
; [17] Reference a BSS-section symbol in the data section

[BITS 32]
[GLOBAL lrotate]	; [1]
[GLOBAL greet]		; [1]
[GLOBAL asmstr]		; [2]
[GLOBAL textptr]	; [2]
[GLOBAL selfptr]	; [2]
[GLOBAL integer]	; [3]
[EXTERN printf]		; [10]
[COMMON commvar 4]	; [7]

[SECTION .text]

; prototype: long lrotate(long x, int num);
lrotate:			; [1]
	  push ebp
	  mov ebp,esp
	  mov eax,[ebp+8]
	  mov ecx,[ebp+12]
.label	  rol eax,1		; [4] [8]
	  loop .label		; [9] [12]
	  mov esp,ebp
	  pop ebp
	  ret

; prototype: void greet(void);
greet	  mov eax,[integer]	; [14]
	  inc eax
	  mov [localint],eax	; [14]
	  push dword [commvar]
	  mov eax,[localptr]	; [13]
	  push dword [eax]
	  push dword [integer]	; [1] [14]
	  push dword printfstr	; [13]
	  call printf		; [11]
	  add esp,16
	  ret

[SECTION .data]

; a string
asmstr	  db 'hello, world', 0	; [2]

; a string for Printf
printfstr db "integer==%d, localint==%d, commvar=%d"
	  db 10, 0

; some pointers
localptr  dd localint		; [5] [17]
textptr  dd greet		; [15]
selfptr  dd selfptr		; [16]

[SECTION .bss]

; an integer
integer  resd 1			; [3]

; a local integer
localint  resd 1		; [6]
