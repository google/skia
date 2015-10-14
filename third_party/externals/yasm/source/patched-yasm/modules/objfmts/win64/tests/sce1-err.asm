PROC_FRAME sample1
[pushreg rbp]
[allocstack 040h]
[setframe rbp, 020h]
[savexmm128 xmm7, 020h]
[savereg rsi, 038h]
[savereg rdi, 010h]
;END_PROLOGUE
ENDPROC_FRAME

PROC_FRAME sample2
[pushreg rbp]
[allocstack 040h]
[setframe rbp, 020h]
[savexmm128 xmm7, 020h]
[savereg rsi, 038h]
[savereg rdi, 010h]
;END_PROLOGUE

PROC_FRAME sample3
[pushreg rbp]
[allocstack 040h]
[setframe rbp, 020h]
[savexmm128 xmm7, 020h]
[savereg rsi, 038h]
END_PROLOGUE
[savereg rdi, 010h]

ENDPROC_FRAME

END_PROLOGUE
[savereg rdi, 010h]

PROC_FRAME sample4
[pushreg rbp]

