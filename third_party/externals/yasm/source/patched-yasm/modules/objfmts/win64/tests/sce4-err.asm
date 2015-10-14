; Negatives
PROC_FRAME sample
[allocstack 0-8]
[setframe rbp, 0-4]
[savexmm128 xmm7, 0-16]
[savereg rsi, 0-8]
END_PROLOGUE
ENDPROC_FRAME

; Too positive
PROC_FRAME sample2
[setframe rbp, 248]
END_PROLOGUE
ENDPROC_FRAME

; Misaligned
PROC_FRAME sample3
[allocstack 128-4]
[setframe rbp, 240-4]
[savexmm128 xmm7, 1024+8]
[savereg rsi, 1024+4]
END_PROLOGUE
ENDPROC_FRAME

