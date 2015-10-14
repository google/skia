; test source file for assembling to ELF shared library
; build with:
;    nasm -f elf elfso.asm
;    ld -shared -o elfso.so elfso.o
; test with:
;    gcc -o elfso elftest.c ./elfso.so
;    ./elfso
; (assuming your gcc is ELF, and you're running bash)

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

	  BITS 32
	  GLOBAL lrotate:function ; [1]
	  GLOBAL greet:function	; [1]
	  GLOBAL asmstr:data asmstr.end-asmstr ; [2]
	  GLOBAL textptr:data 4	; [2]
	  GLOBAL selfptr:data 4	; [2]
	  GLOBAL integer:data 4	; [3]
	  EXTERN printf		; [10]
	  COMMON commvar 4:4	; [7]
	  EXTERN _GLOBAL_OFFSET_TABLE_

	  SECTION .text

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
greet	  push ebx		; we'll use EBX for GOT, so save it
	  call .getgot
.getgot:  pop ebx
	  add ebx,_GLOBAL_OFFSET_TABLE_ + $$ - .getgot wrt ..gotpc
	  mov eax,[ebx+integer wrt ..got] ; [14]
	  mov eax,[eax]
	  inc eax
	  mov [ebx+localint wrt ..gotoff],eax ; [14]
	  mov eax,[ebx+commvar wrt ..got]
	  push dword [eax]
	  mov eax,[ebx+localptr wrt ..gotoff] ; [13]
	  push dword [eax]
	  mov eax,[ebx+integer wrt ..got] ; [1] [14]
	  push dword [eax]
	  lea eax,[ebx+printfstr wrt ..gotoff]
	  push eax		; [13]
	  call printf wrt ..plt	; [11]
	  add esp,16
	  pop ebx
	  ret

	  SECTION .data

; a string
asmstr	  db 'hello, world', 0	; [2]
.end

; a string for Printf
printfstr db "integer==%d, localint==%d, commvar=%d"
	  db 10, 0

; some pointers
localptr  dd localint		; [5] [17]
textptr	  dd greet wrt ..sym	; [15]
selfptr	  dd selfptr wrt ..sym	; [16]

	  SECTION .bss

; an integer
integer	  resd 1		; [3]

; a local integer
localint  resd 1		; [6]
