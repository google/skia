; test source file for assembling to binary files
; build with:
;    yasm -f bin -o bintest.com bintest.asm

; When run (as a DOS .COM file), this program should print
;    hello, world
; on two successive lines, then exit cleanly.

; This file should test the following:
; [1] Define a text-section symbol
; [2] Define a data-section symbol
; [3] Define a BSS-section symbol
; [4] Define a NASM local label
; [5] Reference a NASM local label
; [6] Reference a text-section symbol in the text section
; [7] Reference a data-section symbol in the text section
; [8] Reference a BSS-section symbol in the text section
; [9] Reference a text-section symbol in the data section
; [10] Reference a data-section symbol in the data section
; [11] Reference a BSS-section symbol in the data section

[BITS 16]
[ORG 0x100]

[SECTION .text]

	  jmp start		; [6]

endX	  mov ax,0x4c00		; [1]
	  int 0x21

start	  mov byte [bss_sym],',' ; [1] [8]
	  mov bx,[bssptr]	; [7]
	  mov al,[bx]
	  mov bx,[dataptr]	; [7]
	  mov [bx],al
	  mov cx,2
.loop	  mov dx,datasym	; [1] [4] [7]
	  mov ah,9
	  push cx
	  int 0x21
	  pop cx
	  loop .loop		; [5] [6]
	  mov bx,[textptr]	; [7]
	  jmp bx

[SECTION .data]

datasym	  db 'hello  world', 13, 10, '$' ; [2]
bssptr	  dw bss_sym		; [2] [11]
dataptr	  dw datasym+5		; [2] [10]
textptr	  dw endX		; [2] [9]

[SECTION .bss]

bss_sym	  resb 1		; [3]
