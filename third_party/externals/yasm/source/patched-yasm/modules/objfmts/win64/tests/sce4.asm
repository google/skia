PROC_FRAME sample
[allocstack 8]			; smallest value
[setframe rbp, 0]		; smallest value
[savexmm128 xmm7, 16*64*1024-16]; last smaller-sized
[savereg rsi, 8*64*1024-8]	; last smaller-sized
END_PROLOGUE
ENDPROC_FRAME

PROC_FRAME sample2
[allocstack 128]		; last smaller-sized
[setframe rbp, 240]		; largest value
[savexmm128 xmm7, 16*64*1024]	; first larger-sized
[savereg rsi, 8*64*1024]	; first larger-sized
END_PROLOGUE
ENDPROC_FRAME

PROC_FRAME sample3
[allocstack 136]		; first medium-sized
END_PROLOGUE
ENDPROC_FRAME

PROC_FRAME sample4
[allocstack 8*64*1024-8]	; last medium-sized
END_PROLOGUE
ENDPROC_FRAME

PROC_FRAME sample5
[allocstack 8*64*1024]		; first larger-sized
END_PROLOGUE
ENDPROC_FRAME
