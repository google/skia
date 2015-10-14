[bits 32]
compd xmm1, xmm4, xmm7, 5			; 0F 25 2D 347 10 05
compd xmm2, xmm5, [0], byte 5			; 0F 25 2D 055 20 00 00 00 00 05
compd xmm3, xmm6, dqword [ebx+ecx*4], byte 5	; 0F 25 2D 064 213 30 05

[bits 64]
compd xmm8, xmm11, xmm3, 5			; 0F 25 2D 333 84 05
compd xmm12, xmm4, xmm14, 5			; 0F 25 2D 346 C1 05
compd xmm9, xmm12, [0], byte 5			; 0F 25 2D 044 045 94 00 00 00 00 05
compd xmm9, xmm12, [r8], byte 5			; 0F 25 2D 040 95 05
compd xmm10, xmm13, dqword [rbx+r9*4], 5	; 0F 25 2D 054 213 A6 05

