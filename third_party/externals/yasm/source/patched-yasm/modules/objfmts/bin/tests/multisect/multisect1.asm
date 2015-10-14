[BITS 32]

[MAP all]
; This file is loaded as a DOS .COM file.
[SEGMENT _TEXT start=0 vstart=100h]
 
; shrink & relocate stack:
	mov	sp, stack_ends   ; NASM puts 460h here -
                                  ; 9B0h is desired.
	mov	bx, sp
	mov	cl, 4
	shr	bx, cl
	mov	ah, 4Ah  ; DOS resize mem.block
	int 21h
 
[SEGMENT GATESEG align=1 follows=_TEXT vstart=0]
; label to use for copying this segment at run-time.
gate0cpy:
dd 0
 
; 32-bit ring-0 protected mode code that interacts
; with the VMM (Win3.x/9x kernel).  To be relocated
; at run-time to memory dynamically allocated with
; DPMI, and called through a call-gate from ring-3.
; vstart=0 makes some calculations easier.
 
; Reserve space for stack:
[SEGMENT .bss follows=GATESEG align=16]
    resb 400h
stack_ends:

