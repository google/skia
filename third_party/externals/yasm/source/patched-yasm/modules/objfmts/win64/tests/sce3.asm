PROC_FRAME sample
   db      048h; emit a REX prefix, to enable hot-patching
push rbp
[pushreg rbp]
sub rsp, 040h
[allocstack 040h]
lea rbp, [rsp+020h]
[setframe rbp, 020h]
movdqa [rbp], xmm7
[savexmm128 xmm7, 020h];the offset is from the base of the frame
;not the scaled offset of the frame
mov [rbp+018h], rsi
[savereg rsi, 018h]
mov [rsp+010h], rdi
[savereg rdi, 010h]; you can still use RSP as the base of the frame
; or any other register you choose
END_PROLOGUE

; you can modify the stack pointer outside of the prologue (similar to alloca)
; because we have a frame pointer.
; if we didn't have a frame pointer, this would be illegal
; if we didn't make this modification,
; there would be no need for a frame pointer

sub rsp, 060h

; we can unwind from the following AV because of the frame pointer

mov rax, 0
mov rax, [rax] ; AV!

; restore the registers that weren't saved with a push
; this isn't part of the official epilog, as described in section 2.5

movdqa xmm7, [rbp]
mov rsi, [rbp+018h]
mov rdi, [rbp-010h]

; Here's the official epilog

lea rsp, [rbp-020h]
pop rbp
ret
ENDPROC_FRAME
struc kFrame
.Fill     resq 1	; fill to 8 mod 16 
.SavedRdi resq 1	; saved register RDI 
.SavedRsi resq 1	; saved register RSI 
endstruc

struc sampleFrame
.Fill     resq 1	; fill to 8 mod 16
.SavedRdi resq 1	; Saved Register RDI 
.SavedRsi resq 1	; Saved Register RSI
endstruc

PROC_FRAME sample2
alloc_stack sampleFrame_size
save_reg rdi, sampleFrame.SavedRdi
save_reg rsi, sampleFrame.SavedRsi
END_PROLOGUE

; function body

mov rsi, [rsp+sampleFrame.SavedRsi]
mov rdi, [rsp+sampleFrame.SavedRdi]

; Here's the official epilog

add rsp, sampleFrame_size
ret
ENDPROC_FRAME

