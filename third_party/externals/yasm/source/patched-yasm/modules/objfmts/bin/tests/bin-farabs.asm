keybuf equ 0040h:001Eh

absolute 5000h
label:

section .text
absval equ 1000h

org 0x100
; Using seg should yield the segment part.
mov ax, seg keybuf
mov ax, seg (0040h:001Eh)	; NASM doesn't understand this syntax
mov es, ax

; Use without seg should yield just the offset part.
mov bx, keybuf
;mov bx, 0040h:001Eh		; Illegal

; Each of the below pairs should be equivalent (and legal) in Yasm.
; There are some differences from NASM here!

; Defaults to near jump (on both NASM and Yasm)
jmp keybuf

; Direct far jump.
jmp 0040h:001Eh

; Force near (non-far) jump (just offset, no segment).
jmp near keybuf
jmp near 0040h:001Eh	; Illegal in NASM ("mismatch in operand sizes")

; A couple of jumps to "normal" absolute addresses.
jmp 0x1e
jmp 0
jmp absval
jmp label

; Non-absolute jump
label2:
jmp label2

; Non-relative access
mov ax, [0]
mov ax, [label]

; Direct far, explicitly.
jmp far keybuf		; Illegal in NASM ("value referenced by FAR is not relocatable")
jmp far 0040h:001Eh	; Illegal in NASM ("mismatch in operand sizes")

keybufptr:
dw keybuf	; offset part
dw seg keybuf	; segment part

