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

