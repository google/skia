; This code is UNTESTED, and almost certainly DOES NOT WORK!
; Do NOT use this as an example of how to write AMD64 shared libraries!
; This code is simply to test the AMD64 ELF WRT relocations.

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

	  [BITS 64]
	  [GLOBAL lrotate:function] ; [1]
	  [GLOBAL greet:function]	; [1]
	  [GLOBAL asmstr:data asmstr.end-asmstr] ; [2]
	  [GLOBAL textptr:data 4]	; [2]
	  [GLOBAL selfptr:data 4]	; [2]
	  [GLOBAL integer:data 4]	; [3]
	  [EXTERN printf]		; [10]
	  [COMMON commvar 4:4]	; [7]
	  [EXTERN _GLOBAL_OFFSET_TABLE_]

	  [SECTION .text]

; prototype: long lrotate(long x, int num);
lrotate:			; [1]
	  push rbp
	  mov rbp,rsp
	  mov rax,[rbp+8]
	  mov rcx,[rbp+12]
.label	  rol rax,1		; [4] [8]
	  loop .label		; [9] [12]
	  mov rsp,rbp
	  pop rbp
	  ret

; prototype: void greet(void);
greet	  push rbx		; we'll use RBX for GOT, so save it
	  mov rbx,[integer wrt ..gotpcrel wrt rip]
	  mov rax,[rbx] ; [14]
	  inc rax
	  mov rbx,[_GLOBAL_OFFSET_TABLE_ wrt ..gotpcrel wrt rip]
	  mov [rbx+localint wrt ..got],eax ; [14]
	  mov rax,[rbx+commvar wrt ..got]
	  push qword [rax]
	  mov rax,[rbx+localptr wrt ..got] ; [13]
	  push qword [rax]
	  mov rax,[rbx+integer wrt ..got] ; [1] [14]
	  push qword [rax]
	  lea rax,[rbx+printfstr wrt ..got]
	  push rax		; [13]
	  call printf wrt ..plt	; [11]
	  add rsp,16
	  pop rbx
	  ret

	  [SECTION .data]

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

	  [SECTION .bss]

; an integer
integer	  resd 1		; [3]

; a local integer
localint  resd 1		; [6]
