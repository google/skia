; Instructions are ordered in SSE5 databook order
; BITS=16 to minimize output length
[bits 16]
compd xmm1, xmm4, xmm7, 5		; 0F 25 2D 347 10 05
compd xmm2, xmm5, [0], byte 5		; 0F 25 2D 056 20 00 00 05
compd xmm3, xmm6, dqword [0], 5		; 0F 25 2D 066 30 00 00 05

comps xmm1, xmm4, xmm7, 5		; 0F 25 2C 347 10 05
comps xmm2, xmm5, [0], byte 5		; 0F 25 2C 056 20 00 00 05
comps xmm3, xmm6, dqword [0], 5		; 0F 25 2C 066 30 00 00 05

comsd xmm1, xmm4, xmm7, 5		; 0F 25 2F 347 10 05
comsd xmm2, xmm5, [0], byte 5		; 0F 25 2F 056 20 00 00 05
comsd xmm3, xmm6, qword [0], 5		; 0F 25 2F 066 30 00 00 05

comss xmm1, xmm4, xmm7, 5		; 0F 25 2E 347 10 05
comss xmm2, xmm5, [0], byte 5		; 0F 25 2E 056 20 00 00 05
comss xmm3, xmm6, dword [0], 5		; 0F 25 2E 066 30 00 00 05

cvtph2ps xmm1, xmm4			; 0F 7A 30 314
cvtph2ps xmm2, [0]			; 0F 7A 30 026 00 00
cvtph2ps xmm3, qword [0]		; 0F 7A 30 036 00 00

cvtps2ph xmm1, xmm4			; 0F 7A 31 341
cvtps2ph [0], xmm2			; 0F 7A 31 026 00 00
cvtps2ph qword [0], xmm3		; 0F 7A 31 036 00 00

fmaddpd xmm1, xmm1, xmm2, xmm3		; 0F 24 01 323 10 /or/ 0F 24 01 332 18
fmaddpd xmm1, xmm1, xmm2, [0]		; 0F 24 01 026 10 00 00
fmaddpd xmm1, xmm1, xmm2, dqword [0]	; 0F 24 01 026 10 00 00
fmaddpd xmm1, xmm1, [0], xmm3		; 0F 24 01 036 18 00 00
fmaddpd xmm1, xmm1, dqword [0], xmm3	; 0F 24 01 036 18 00 00
fmaddpd xmm1, xmm2, xmm3, xmm1		; 0F 24 05 323 10 /or/ 0F 24 05 332 18
fmaddpd xmm1, xmm2, [0], xmm1		; 0F 24 05 026 10 00 00
fmaddpd xmm1, xmm2, dqword [0], xmm1	; 0F 24 05 026 10 00 00
fmaddpd xmm1, [0], xmm3, xmm1		; 0F 24 05 036 18 00 00
fmaddpd xmm1, dqword [0], xmm3, xmm1	; 0F 24 05 036 18 00 00

fmaddps xmm1, xmm1, xmm2, xmm3		; 0F 24 00 323 10 /or/ 0F 24 00 332 18
fmaddps xmm1, xmm1, xmm2, [0]		; 0F 24 00 026 10 00 00
fmaddps xmm1, xmm1, xmm2, dqword [0]	; 0F 24 00 026 10 00 00
fmaddps xmm1, xmm1, [0], xmm3		; 0F 24 00 036 18 00 00
fmaddps xmm1, xmm1, dqword [0], xmm3	; 0F 24 00 036 18 00 00
fmaddps xmm1, xmm2, xmm3, xmm1		; 0F 24 04 323 10 /or/ 0F 24 04 332 18
fmaddps xmm1, xmm2, [0], xmm1		; 0F 24 04 026 10 00 00
fmaddps xmm1, xmm2, dqword [0], xmm1	; 0F 24 04 026 10 00 00
fmaddps xmm1, [0], xmm3, xmm1		; 0F 24 04 036 18 00 00
fmaddps xmm1, dqword [0], xmm3, xmm1	; 0F 24 04 036 18 00 00

fmaddsd xmm1, xmm1, xmm2, xmm3		; 0F 24 03 323 10 /or/ 0F 24 03 332 18
fmaddsd xmm1, xmm1, xmm2, [0]		; 0F 24 03 026 10 00 00
fmaddsd xmm1, xmm1, xmm2, qword [0]	; 0F 24 03 026 10 00 00
fmaddsd xmm1, xmm1, [0], xmm3		; 0F 24 03 036 18 00 00
fmaddsd xmm1, xmm1, qword [0], xmm3	; 0F 24 03 036 18 00 00
fmaddsd xmm1, xmm2, xmm3, xmm1		; 0F 24 07 323 10 /or/ 0F 24 07 332 18
fmaddsd xmm1, xmm2, [0], xmm1		; 0F 24 07 026 10 00 00
fmaddsd xmm1, xmm2, qword [0], xmm1	; 0F 24 07 026 10 00 00
fmaddsd xmm1, [0], xmm3, xmm1		; 0F 24 07 036 18 00 00
fmaddsd xmm1, qword [0], xmm3, xmm1	; 0F 24 07 036 18 00 00

fmaddss xmm1, xmm1, xmm2, xmm3		; 0F 24 02 323 10 /or/ 0F 24 02 332 18
fmaddss xmm1, xmm1, xmm2, [0]		; 0F 24 02 026 10 00 00
fmaddss xmm1, xmm1, xmm2, dword [0]	; 0F 24 02 026 10 00 00
fmaddss xmm1, xmm1, [0], xmm3		; 0F 24 02 036 18 00 00
fmaddss xmm1, xmm1, dword [0], xmm3	; 0F 24 02 036 18 00 00
fmaddss xmm1, xmm2, xmm3, xmm1		; 0F 24 06 323 10 /or/ 0F 24 06 332 18
fmaddss xmm1, xmm2, [0], xmm1		; 0F 24 06 026 10 00 00
fmaddss xmm1, xmm2, dword [0], xmm1	; 0F 24 06 026 10 00 00
fmaddss xmm1, [0], xmm3, xmm1		; 0F 24 06 036 18 00 00
fmaddss xmm1, dword [0], xmm3, xmm1	; 0F 24 06 036 18 00 00

fmsubpd xmm1, xmm1, xmm2, xmm3		; 0F 24 09 323 10 /or/ 0F 24 09 332 18
fmsubpd xmm1, xmm1, xmm2, [0]		; 0F 24 09 026 10 00 00
fmsubpd xmm1, xmm1, xmm2, dqword [0]	; 0F 24 09 026 10 00 00
fmsubpd xmm1, xmm1, [0], xmm3		; 0F 24 09 036 18 00 00
fmsubpd xmm1, xmm1, dqword [0], xmm3	; 0F 24 09 036 18 00 00
fmsubpd xmm1, xmm2, xmm3, xmm1		; 0F 24 0D 323 10 /or/ 0F 24 0D 332 18
fmsubpd xmm1, xmm2, [0], xmm1		; 0F 24 0D 026 10 00 00
fmsubpd xmm1, xmm2, dqword [0], xmm1	; 0F 24 0D 026 10 00 00
fmsubpd xmm1, [0], xmm3, xmm1		; 0F 24 0D 036 18 00 00
fmsubpd xmm1, dqword [0], xmm3, xmm1	; 0F 24 0D 036 18 00 00

fmsubps xmm1, xmm1, xmm2, xmm3		; 0F 24 08 323 10 /or/ 0F 24 08 332 18
fmsubps xmm1, xmm1, xmm2, [0]		; 0F 24 08 026 10 00 00
fmsubps xmm1, xmm1, xmm2, dqword [0]	; 0F 24 08 026 10 00 00
fmsubps xmm1, xmm1, [0], xmm3		; 0F 24 08 036 18 00 00
fmsubps xmm1, xmm1, dqword [0], xmm3	; 0F 24 08 036 18 00 00
fmsubps xmm1, xmm2, xmm3, xmm1		; 0F 24 0C 323 10 /or/ 0F 24 0C 332 18
fmsubps xmm1, xmm2, [0], xmm1		; 0F 24 0C 026 10 00 00
fmsubps xmm1, xmm2, dqword [0], xmm1	; 0F 24 0C 026 10 00 00
fmsubps xmm1, [0], xmm3, xmm1		; 0F 24 0C 036 18 00 00
fmsubps xmm1, dqword [0], xmm3, xmm1	; 0F 24 0C 036 18 00 00

fmsubsd xmm1, xmm1, xmm2, xmm3		; 0F 24 0B 323 10 /or/ 0F 24 0B 332 18
fmsubsd xmm1, xmm1, xmm2, [0]		; 0F 24 0B 026 10 00 00
fmsubsd xmm1, xmm1, xmm2, qword [0]	; 0F 24 0B 026 10 00 00
fmsubsd xmm1, xmm1, [0], xmm3		; 0F 24 0B 036 18 00 00
fmsubsd xmm1, xmm1, qword [0], xmm3	; 0F 24 0B 036 18 00 00
fmsubsd xmm1, xmm2, xmm3, xmm1		; 0F 24 0F 323 10 /or/ 0F 24 0F 332 18
fmsubsd xmm1, xmm2, [0], xmm1		; 0F 24 0F 026 10 00 00
fmsubsd xmm1, xmm2, qword [0], xmm1	; 0F 24 0F 026 10 00 00
fmsubsd xmm1, [0], xmm3, xmm1		; 0F 24 0F 036 18 00 00
fmsubsd xmm1, qword [0], xmm3, xmm1	; 0F 24 0F 036 18 00 00

fmsubss xmm1, xmm1, xmm2, xmm3		; 0F 24 0A 323 10 /or/ 0F 24 0A 332 18
fmsubss xmm1, xmm1, xmm2, [0]		; 0F 24 0A 026 10 00 00
fmsubss xmm1, xmm1, xmm2, dword [0]	; 0F 24 0A 026 10 00 00
fmsubss xmm1, xmm1, [0], xmm3		; 0F 24 0A 036 18 00 00
fmsubss xmm1, xmm1, dword [0], xmm3	; 0F 24 0A 036 18 00 00
fmsubss xmm1, xmm2, xmm3, xmm1		; 0F 24 0E 323 10 /or/ 0F 24 0E 332 18
fmsubss xmm1, xmm2, [0], xmm1		; 0F 24 0E 026 10 00 00
fmsubss xmm1, xmm2, dword [0], xmm1	; 0F 24 0E 026 10 00 00
fmsubss xmm1, [0], xmm3, xmm1		; 0F 24 0E 036 18 00 00
fmsubss xmm1, dword [0], xmm3, xmm1	; 0F 24 0E 036 18 00 00

fnmaddpd xmm1, xmm1, xmm2, xmm3		; 0F 24 11 323 10 /or/ 0F 24 11 332 18
fnmaddpd xmm1, xmm1, xmm2, [0]		; 0F 24 11 026 10 00 00
fnmaddpd xmm1, xmm1, xmm2, dqword [0]	; 0F 24 11 026 10 00 00
fnmaddpd xmm1, xmm1, [0], xmm3		; 0F 24 11 036 18 00 00
fnmaddpd xmm1, xmm1, dqword [0], xmm3	; 0F 24 11 036 18 00 00
fnmaddpd xmm1, xmm2, xmm3, xmm1		; 0F 24 15 323 10 /or/ 0F 24 15 332 18
fnmaddpd xmm1, xmm2, [0], xmm1		; 0F 24 15 026 10 00 00
fnmaddpd xmm1, xmm2, dqword [0], xmm1	; 0F 24 15 026 10 00 00
fnmaddpd xmm1, [0], xmm3, xmm1		; 0F 24 15 036 18 00 00
fnmaddpd xmm1, dqword [0], xmm3, xmm1	; 0F 24 15 036 18 00 00

fnmaddps xmm1, xmm1, xmm2, xmm3		; 0F 24 10 323 10 /or/ 0F 24 10 332 18
fnmaddps xmm1, xmm1, xmm2, [0]		; 0F 24 10 026 10 00 00
fnmaddps xmm1, xmm1, xmm2, dqword [0]	; 0F 24 10 026 10 00 00
fnmaddps xmm1, xmm1, [0], xmm3		; 0F 24 10 036 18 00 00
fnmaddps xmm1, xmm1, dqword [0], xmm3	; 0F 24 10 036 18 00 00
fnmaddps xmm1, xmm2, xmm3, xmm1		; 0F 24 14 323 10 /or/ 0F 24 14 332 18
fnmaddps xmm1, xmm2, [0], xmm1		; 0F 24 14 026 10 00 00
fnmaddps xmm1, xmm2, dqword [0], xmm1	; 0F 24 14 026 10 00 00
fnmaddps xmm1, [0], xmm3, xmm1		; 0F 24 14 036 18 00 00
fnmaddps xmm1, dqword [0], xmm3, xmm1	; 0F 24 14 036 18 00 00

fnmaddsd xmm1, xmm1, xmm2, xmm3		; 0F 24 13 323 10 /or/ 0F 24 13 332 18
fnmaddsd xmm1, xmm1, xmm2, [0]		; 0F 24 13 026 10 00 00
fnmaddsd xmm1, xmm1, xmm2, qword [0]	; 0F 24 13 026 10 00 00
fnmaddsd xmm1, xmm1, [0], xmm3		; 0F 24 13 036 18 00 00
fnmaddsd xmm1, xmm1, qword [0], xmm3	; 0F 24 13 036 18 00 00
fnmaddsd xmm1, xmm2, xmm3, xmm1		; 0F 24 17 323 10 /or/ 0F 24 17 332 18
fnmaddsd xmm1, xmm2, [0], xmm1		; 0F 24 17 026 10 00 00
fnmaddsd xmm1, xmm2, qword [0], xmm1	; 0F 24 17 026 10 00 00
fnmaddsd xmm1, [0], xmm3, xmm1		; 0F 24 17 036 18 00 00
fnmaddsd xmm1, qword [0], xmm3, xmm1	; 0F 24 17 036 18 00 00

fnmaddss xmm1, xmm1, xmm2, xmm3		; 0F 24 12 323 10 /or/ 0F 24 12 332 18
fnmaddss xmm1, xmm1, xmm2, [0]		; 0F 24 12 026 10 00 00
fnmaddss xmm1, xmm1, xmm2, dword [0]	; 0F 24 12 026 10 00 00
fnmaddss xmm1, xmm1, [0], xmm3		; 0F 24 12 036 18 00 00
fnmaddss xmm1, xmm1, dword [0], xmm3	; 0F 24 12 036 18 00 00
fnmaddss xmm1, xmm2, xmm3, xmm1		; 0F 24 16 323 10 /or/ 0F 24 16 332 18
fnmaddss xmm1, xmm2, [0], xmm1		; 0F 24 16 026 10 00 00
fnmaddss xmm1, xmm2, dword [0], xmm1	; 0F 24 16 026 10 00 00
fnmaddss xmm1, [0], xmm3, xmm1		; 0F 24 16 036 18 00 00
fnmaddss xmm1, dword [0], xmm3, xmm1	; 0F 24 16 036 18 00 00

fnmsubpd xmm1, xmm1, xmm2, xmm3		; 0F 24 19 323 10 /or/ 0F 24 19 332 18
fnmsubpd xmm1, xmm1, xmm2, [0]		; 0F 24 19 026 10 00 00
fnmsubpd xmm1, xmm1, xmm2, dqword [0]	; 0F 24 19 026 10 00 00
fnmsubpd xmm1, xmm1, [0], xmm3		; 0F 24 19 036 18 00 00
fnmsubpd xmm1, xmm1, dqword [0], xmm3	; 0F 24 19 036 18 00 00
fnmsubpd xmm1, xmm2, xmm3, xmm1		; 0F 24 1D 323 10 /or/ 0F 24 1D 332 18
fnmsubpd xmm1, xmm2, [0], xmm1		; 0F 24 1D 026 10 00 00
fnmsubpd xmm1, xmm2, dqword [0], xmm1	; 0F 24 1D 026 10 00 00
fnmsubpd xmm1, [0], xmm3, xmm1		; 0F 24 1D 036 18 00 00
fnmsubpd xmm1, dqword [0], xmm3, xmm1	; 0F 24 1D 036 18 00 00

fnmsubps xmm1, xmm1, xmm2, xmm3		; 0F 24 18 323 10 /or/ 0F 24 18 332 18
fnmsubps xmm1, xmm1, xmm2, [0]		; 0F 24 18 026 10 00 00
fnmsubps xmm1, xmm1, xmm2, dqword [0]	; 0F 24 18 026 10 00 00
fnmsubps xmm1, xmm1, [0], xmm3		; 0F 24 18 036 18 00 00
fnmsubps xmm1, xmm1, dqword [0], xmm3	; 0F 24 18 036 18 00 00
fnmsubps xmm1, xmm2, xmm3, xmm1		; 0F 24 1C 323 10 /or/ 0F 24 1C 332 18
fnmsubps xmm1, xmm2, [0], xmm1		; 0F 24 1C 026 10 00 00
fnmsubps xmm1, xmm2, dqword [0], xmm1	; 0F 24 1C 026 10 00 00
fnmsubps xmm1, [0], xmm3, xmm1		; 0F 24 1C 036 18 00 00
fnmsubps xmm1, dqword [0], xmm3, xmm1	; 0F 24 1C 036 18 00 00

fnmsubsd xmm1, xmm1, xmm2, xmm3		; 0F 24 1B 323 10 /or/ 0F 24 1B 332 18
fnmsubsd xmm1, xmm1, xmm2, [0]		; 0F 24 1B 026 10 00 00
fnmsubsd xmm1, xmm1, xmm2, qword [0]	; 0F 24 1B 026 10 00 00
fnmsubsd xmm1, xmm1, [0], xmm3		; 0F 24 1B 036 18 00 00
fnmsubsd xmm1, xmm1, qword [0], xmm3	; 0F 24 1B 036 18 00 00
fnmsubsd xmm1, xmm2, xmm3, xmm1		; 0F 24 1F 323 10 /or/ 0F 24 1F 332 18
fnmsubsd xmm1, xmm2, [0], xmm1		; 0F 24 1F 026 10 00 00
fnmsubsd xmm1, xmm2, qword [0], xmm1	; 0F 24 1F 026 10 00 00
fnmsubsd xmm1, [0], xmm3, xmm1		; 0F 24 1F 036 18 00 00
fnmsubsd xmm1, qword [0], xmm3, xmm1	; 0F 24 1F 036 18 00 00

fnmsubss xmm1, xmm1, xmm2, xmm3		; 0F 24 1A 323 10 /or/ 0F 24 1A 332 18
fnmsubss xmm1, xmm1, xmm2, [0]		; 0F 24 1A 026 10 00 00
fnmsubss xmm1, xmm1, xmm2, dword [0]	; 0F 24 1A 026 10 00 00
fnmsubss xmm1, xmm1, [0], xmm3		; 0F 24 1A 036 18 00 00
fnmsubss xmm1, xmm1, dword [0], xmm3	; 0F 24 1A 036 18 00 00
fnmsubss xmm1, xmm2, xmm3, xmm1		; 0F 24 1E 323 10 /or/ 0F 24 1E 332 18
fnmsubss xmm1, xmm2, [0], xmm1		; 0F 24 1E 026 10 00 00
fnmsubss xmm1, xmm2, dword [0], xmm1	; 0F 24 1E 026 10 00 00
fnmsubss xmm1, [0], xmm3, xmm1		; 0F 24 1E 036 18 00 00
fnmsubss xmm1, dword [0], xmm3, xmm1	; 0F 24 1E 036 18 00 00

frczpd xmm1, xmm2			; 0F 7A 11 312
frczpd xmm1, [0]			; 0F 7A 11 016 00 00
frczpd xmm1, dqword [0]			; 0F 7A 11 016 00 00

frczps xmm1, xmm2			; 0F 7A 10 312
frczps xmm1, [0]			; 0F 7A 10 016 00 00
frczps xmm1, dqword [0]			; 0F 7A 10 016 00 00

frczsd xmm1, xmm2			; 0F 7A 13 312
frczsd xmm1, [0]			; 0F 7A 13 016 00 00
frczsd xmm1, qword [0]			; 0F 7A 13 016 00 00

frczss xmm1, xmm2			; 0F 7A 12 312
frczss xmm1, [0]			; 0F 7A 12 016 00 00
frczss xmm1, dword [0]			; 0F 7A 12 016 00 00

pcmov xmm1, xmm1, xmm2, xmm3		; 0F 24 22 323 10 /or/ 0F 24 22 332 18
pcmov xmm1, xmm1, xmm2, [0]		; 0F 24 22 026 10 00 00
pcmov xmm1, xmm1, xmm2, dqword [0]	; 0F 24 22 026 10 00 00
pcmov xmm1, xmm1, [0], xmm3		; 0F 24 22 036 18 00 00
pcmov xmm1, xmm1, dqword [0], xmm3	; 0F 24 22 036 18 00 00
pcmov xmm1, xmm2, xmm3, xmm1		; 0F 24 26 323 10 /or/ 0F 24 26 332 18
pcmov xmm1, xmm2, [0], xmm1		; 0F 24 26 026 10 00 00
pcmov xmm1, xmm2, dqword [0], xmm1	; 0F 24 26 026 10 00 00
pcmov xmm1, [0], xmm3, xmm1		; 0F 24 26 036 18 00 00
pcmov xmm1, dqword [0], xmm3, xmm1	; 0F 24 26 036 18 00 00

pcomb xmm1, xmm4, xmm7, 5		; 0F 25 4C 347 10 05
pcomb xmm2, xmm5, [0], byte 5		; 0F 25 4C 056 20 00 00 05
pcomb xmm3, xmm6, dqword [0], 5		; 0F 25 4C 066 30 00 00 05

pcomd xmm1, xmm4, xmm7, 5		; 0F 25 4E 347 10 05
pcomd xmm2, xmm5, [0], byte 5		; 0F 25 4E 056 20 00 00 05
pcomd xmm3, xmm6, dqword [0], 5		; 0F 25 4E 066 30 00 00 05

pcomq xmm1, xmm4, xmm7, 5		; 0F 25 4F 347 10 05
pcomq xmm2, xmm5, [0], byte 5		; 0F 25 4F 056 20 00 00 05
pcomq xmm3, xmm6, dqword [0], 5		; 0F 25 4F 066 30 00 00 05

pcomub xmm1, xmm4, xmm7, 5		; 0F 25 6C 347 10 05
pcomub xmm2, xmm5, [0], byte 5		; 0F 25 6C 056 20 00 00 05
pcomub xmm3, xmm6, dqword [0], 5	; 0F 25 6C 066 30 00 00 05

pcomud xmm1, xmm4, xmm7, 5		; 0F 25 6E 347 10 05
pcomud xmm2, xmm5, [0], byte 5		; 0F 25 6E 056 20 00 00 05
pcomud xmm3, xmm6, dqword [0], 5	; 0F 25 6E 066 30 00 00 05

pcomuq xmm1, xmm4, xmm7, 5		; 0F 25 6F 347 10 05
pcomuq xmm2, xmm5, [0], byte 5		; 0F 25 6F 056 20 00 00 05
pcomuq xmm3, xmm6, dqword [0], 5	; 0F 25 6F 066 30 00 00 05

pcomuw xmm1, xmm4, xmm7, 5		; 0F 25 6D 347 10 05
pcomuw xmm2, xmm5, [0], byte 5		; 0F 25 6D 056 20 00 00 05
pcomuw xmm3, xmm6, dqword [0], 5	; 0F 25 6D 066 30 00 00 05

pcomw xmm1, xmm4, xmm7, 5		; 0F 25 4D 347 10 05
pcomw xmm2, xmm5, [0], byte 5		; 0F 25 4D 056 20 00 00 05
pcomw xmm3, xmm6, dqword [0], 5		; 0F 25 4D 066 30 00 00 05

permpd xmm1, xmm1, xmm2, xmm3		; 0F 24 21 323 10 /or/ 0F 24 21 332 18
permpd xmm1, xmm1, xmm2, [0]		; 0F 24 21 026 10 00 00
permpd xmm1, xmm1, xmm2, dqword [0]	; 0F 24 21 026 10 00 00
permpd xmm1, xmm1, [0], xmm3		; 0F 24 21 036 18 00 00
permpd xmm1, xmm1, dqword [0], xmm3	; 0F 24 21 036 18 00 00
permpd xmm1, xmm2, xmm3, xmm1		; 0F 24 25 323 10 /or/ 0F 24 25 332 18
permpd xmm1, xmm2, [0], xmm1		; 0F 24 25 026 10 00 00
permpd xmm1, xmm2, dqword [0], xmm1	; 0F 24 25 026 10 00 00
permpd xmm1, [0], xmm3, xmm1		; 0F 24 25 036 18 00 00
permpd xmm1, dqword [0], xmm3, xmm1	; 0F 24 25 036 18 00 00

permps xmm1, xmm1, xmm2, xmm3		; 0F 24 20 323 10 /or/ 0F 24 20 332 18
permps xmm1, xmm1, xmm2, [0]		; 0F 24 20 026 10 00 00
permps xmm1, xmm1, xmm2, dqword [0]	; 0F 24 20 026 10 00 00
permps xmm1, xmm1, [0], xmm3		; 0F 24 20 036 18 00 00
permps xmm1, xmm1, dqword [0], xmm3	; 0F 24 20 036 18 00 00
permps xmm1, xmm2, xmm3, xmm1		; 0F 24 24 323 10 /or/ 0F 24 24 332 18
permps xmm1, xmm2, [0], xmm1		; 0F 24 24 026 10 00 00
permps xmm1, xmm2, dqword [0], xmm1	; 0F 24 24 026 10 00 00
permps xmm1, [0], xmm3, xmm1		; 0F 24 24 036 18 00 00
permps xmm1, dqword [0], xmm3, xmm1	; 0F 24 24 036 18 00 00

phaddbd xmm1, xmm2			; 0F 7A 42 312
phaddbd xmm1, [0]			; 0F 7A 42 016 00 00
phaddbd xmm1, dqword [0]		; 0F 7A 42 016 00 00

phaddbq xmm1, xmm2			; 0F 7A 43 312
phaddbq xmm1, [0]			; 0F 7A 43 016 00 00
phaddbq xmm1, dqword [0]		; 0F 7A 43 016 00 00

phaddbw xmm1, xmm2			; 0F 7A 41 312
phaddbw xmm1, [0]			; 0F 7A 41 016 00 00
phaddbw xmm1, dqword [0]		; 0F 7A 41 016 00 00

phadddq xmm1, xmm2			; 0F 7A 4B 312
phadddq xmm1, [0]			; 0F 7A 4B 016 00 00
phadddq xmm1, dqword [0]		; 0F 7A 4B 016 00 00

phaddubd xmm1, xmm2			; 0F 7A 52 312
phaddubd xmm1, [0]			; 0F 7A 52 016 00 00
phaddubd xmm1, dqword [0]		; 0F 7A 52 016 00 00

phaddubq xmm1, xmm2			; 0F 7A 53 312
phaddubq xmm1, [0]			; 0F 7A 53 016 00 00
phaddubq xmm1, dqword [0]		; 0F 7A 53 016 00 00

phaddubw xmm1, xmm2			; 0F 7A 51 312
phaddubw xmm1, [0]			; 0F 7A 51 016 00 00
phaddubw xmm1, dqword [0]		; 0F 7A 51 016 00 00

phaddudq xmm1, xmm2			; 0F 7A 5B 312
phaddudq xmm1, [0]			; 0F 7A 5B 016 00 00
phaddudq xmm1, dqword [0]		; 0F 7A 5B 016 00 00

phadduwd xmm1, xmm2			; 0F 7A 56 312
phadduwd xmm1, [0]			; 0F 7A 56 016 00 00
phadduwd xmm1, dqword [0]		; 0F 7A 56 016 00 00

phadduwq xmm1, xmm2			; 0F 7A 57 312
phadduwq xmm1, [0]			; 0F 7A 57 016 00 00
phadduwq xmm1, dqword [0]		; 0F 7A 57 016 00 00

phaddwd xmm1, xmm2			; 0F 7A 46 312
phaddwd xmm1, [0]			; 0F 7A 46 016 00 00
phaddwd xmm1, dqword [0]		; 0F 7A 46 016 00 00

phaddwq xmm1, xmm2			; 0F 7A 47 312
phaddwq xmm1, [0]			; 0F 7A 47 016 00 00
phaddwq xmm1, dqword [0]		; 0F 7A 47 016 00 00

phsubbw xmm1, xmm2			; 0F 7A 61 312
phsubbw xmm1, [0]			; 0F 7A 61 016 00 00
phsubbw xmm1, dqword [0]		; 0F 7A 61 016 00 00

phsubdq xmm1, xmm2			; 0F 7A 63 312
phsubdq xmm1, [0]			; 0F 7A 63 016 00 00
phsubdq xmm1, dqword [0]		; 0F 7A 63 016 00 00

phsubwd xmm1, xmm2			; 0F 7A 62 312
phsubwd xmm1, [0]			; 0F 7A 62 016 00 00
phsubwd xmm1, dqword [0]		; 0F 7A 62 016 00 00

pmacsdd xmm1, xmm4, xmm7, xmm1		; 0F 24 9E 347 10
pmacsdd xmm2, xmm5, [0], xmm2		; 0F 24 9E 056 20 00 00
pmacsdd xmm3, xmm6, dqword [0], xmm3	; 0F 24 9E 066 30 00 00

pmacsdqh xmm1, xmm4, xmm7, xmm1		; 0F 24 9F 347 10
pmacsdqh xmm2, xmm5, [0], xmm2		; 0F 24 9F 056 20 00 00
pmacsdqh xmm3, xmm6, dqword [0], xmm3	; 0F 24 9F 066 30 00 00

pmacsdql xmm1, xmm4, xmm7, xmm1		; 0F 24 97 347 10
pmacsdql xmm2, xmm5, [0], xmm2		; 0F 24 97 056 20 00 00
pmacsdql xmm3, xmm6, dqword [0], xmm3	; 0F 24 97 066 30 00 00

pmacssdd xmm1, xmm4, xmm7, xmm1		; 0F 24 8E 347 10
pmacssdd xmm2, xmm5, [0], xmm2		; 0F 24 8E 056 20 00 00
pmacssdd xmm3, xmm6, dqword [0], xmm3	; 0F 24 8E 066 30 00 00

pmacssdqh xmm1, xmm4, xmm7, xmm1	; 0F 24 8F 347 10
pmacssdqh xmm2, xmm5, [0], xmm2		; 0F 24 8F 056 20 00 00
pmacssdqh xmm3, xmm6, dqword [0], xmm3	; 0F 24 8F 066 30 00 00

pmacssdql xmm1, xmm4, xmm7, xmm1	; 0F 24 87 347 10
pmacssdql xmm2, xmm5, [0], xmm2		; 0F 24 87 056 20 00 00
pmacssdql xmm3, xmm6, dqword [0], xmm3	; 0F 24 87 066 30 00 00

pmacsswd xmm1, xmm4, xmm7, xmm1		; 0F 24 86 347 10
pmacsswd xmm2, xmm5, [0], xmm2		; 0F 24 86 056 20 00 00
pmacsswd xmm3, xmm6, dqword [0], xmm3	; 0F 24 86 066 30 00 00

pmacssww xmm1, xmm4, xmm7, xmm1		; 0F 24 85 347 10
pmacssww xmm2, xmm5, [0], xmm2		; 0F 24 85 056 20 00 00
pmacssww xmm3, xmm6, dqword [0], xmm3	; 0F 24 85 066 30 00 00

pmacswd xmm1, xmm4, xmm7, xmm1		; 0F 24 96 347 10
pmacswd xmm2, xmm5, [0], xmm2		; 0F 24 96 056 20 00 00
pmacswd xmm3, xmm6, dqword [0], xmm3	; 0F 24 96 066 30 00 00

pmacsww xmm1, xmm4, xmm7, xmm1		; 0F 24 95 347 10
pmacsww xmm2, xmm5, [0], xmm2		; 0F 24 95 056 20 00 00
pmacsww xmm3, xmm6, dqword [0], xmm3	; 0F 24 95 066 30 00 00

pmadcsswd xmm1, xmm4, xmm7, xmm1	; 0F 24 A6 347 10
pmadcsswd xmm2, xmm5, [0], xmm2		; 0F 24 A6 056 20 00 00
pmadcsswd xmm3, xmm6, dqword [0], xmm3	; 0F 24 A6 066 30 00 00

pmadcswd xmm1, xmm4, xmm7, xmm1		; 0F 24 B6 347 10
pmadcswd xmm2, xmm5, [0], xmm2		; 0F 24 B6 056 20 00 00
pmadcswd xmm3, xmm6, dqword [0], xmm3	; 0F 24 B6 066 30 00 00

pperm xmm1, xmm1, xmm2, xmm3		; 0F 24 23 323 10 /or/ 0F 24 23 332 18
pperm xmm1, xmm1, xmm2, [0]		; 0F 24 23 026 10 00 00
pperm xmm1, xmm1, xmm2, dqword [0]	; 0F 24 23 026 10 00 00
pperm xmm1, xmm1, [0], xmm3		; 0F 24 23 036 18 00 00
pperm xmm1, xmm1, dqword [0], xmm3	; 0F 24 23 036 18 00 00
pperm xmm1, xmm2, xmm3, xmm1		; 0F 24 27 323 10 /or/ 0F 24 27 332 18
pperm xmm1, xmm2, [0], xmm1		; 0F 24 27 026 10 00 00
pperm xmm1, xmm2, dqword [0], xmm1	; 0F 24 27 026 10 00 00
pperm xmm1, [0], xmm3, xmm1		; 0F 24 27 036 18 00 00
pperm xmm1, dqword [0], xmm3, xmm1	; 0F 24 27 036 18 00 00

protb xmm1, xmm2, xmm3			; 0F 24 40 323 10 /or/ 0F 24 40 332 18
protb xmm1, xmm2, [0]			; 0F 24 40 026 10 00 00
protb xmm1, xmm2, dqword [0]		; 0F 24 40 026 10 00 00
protb xmm1, [0], xmm3			; 0F 24 40 036 18 00 00
protb xmm1, dqword [0], xmm3		; 0F 24 40 036 18 00 00
protb xmm1, xmm2, byte 5		; 0F 7B 40 312 05
protb xmm1, [0], byte 5			; 0F 7B 40 016 00 00 05
protb xmm1, dqword [0], 5		; 0F 7B 40 016 00 00 05

protd xmm1, xmm2, xmm3			; 0F 24 42 323 10 /or/ 0F 24 42 332 18
protd xmm1, xmm2, [0]			; 0F 24 42 026 10 00 00
protd xmm1, xmm2, dqword [0]		; 0F 24 42 026 10 00 00
protd xmm1, [0], xmm3			; 0F 24 42 036 18 00 00
protd xmm1, dqword [0], xmm3		; 0F 24 42 036 18 00 00
protd xmm1, xmm2, byte 5		; 0F 7B 42 312 05
protd xmm1, [0], byte 5			; 0F 7B 42 016 00 00 05
protd xmm1, dqword [0], 5		; 0F 7B 42 016 00 00 05

protq xmm1, xmm2, xmm3			; 0F 24 43 323 10 /or/ 0F 24 43 332 18
protq xmm1, xmm2, [0]			; 0F 24 43 026 10 00 00
protq xmm1, xmm2, dqword [0]		; 0F 24 43 026 10 00 00
protq xmm1, [0], xmm3			; 0F 24 43 036 18 00 00
protq xmm1, dqword [0], xmm3		; 0F 24 43 036 18 00 00
protq xmm1, xmm2, byte 5		; 0F 7B 43 312 05
protq xmm1, [0], byte 5			; 0F 7B 43 016 00 00 05
protq xmm1, dqword [0], 5		; 0F 7B 43 016 00 00 05

protw xmm1, xmm2, xmm3			; 0F 24 41 323 10 /or/ 0F 24 41 332 18
protw xmm1, xmm2, [0]			; 0F 24 41 026 10 00 00
protw xmm1, xmm2, dqword [0]		; 0F 24 41 026 10 00 00
protw xmm1, [0], xmm3			; 0F 24 41 036 18 00 00
protw xmm1, dqword [0], xmm3		; 0F 24 41 036 18 00 00
protw xmm1, xmm2, byte 5		; 0F 7B 41 312 05
protw xmm1, [0], byte 5			; 0F 7B 41 016 00 00 05
protw xmm1, dqword [0], 5		; 0F 7B 41 016 00 00 05

pshab xmm1, xmm2, xmm3			; 0F 24 48 323 10 /or/ 0F 24 48 332 18
pshab xmm1, xmm2, [0]			; 0F 24 48 026 10 00 00
pshab xmm1, xmm2, dqword [0]		; 0F 24 48 026 10 00 00
pshab xmm1, [0], xmm3			; 0F 24 48 036 18 00 00
pshab xmm1, dqword [0], xmm3		; 0F 24 48 036 18 00 00

pshad xmm1, xmm2, xmm3			; 0F 24 4A 323 10 /or/ 0F 24 4A 332 18
pshad xmm1, xmm2, [0]			; 0F 24 4A 026 10 00 00
pshad xmm1, xmm2, dqword [0]		; 0F 24 4A 026 10 00 00
pshad xmm1, [0], xmm3			; 0F 24 4A 036 18 00 00
pshad xmm1, dqword [0], xmm3		; 0F 24 4A 036 18 00 00

pshaq xmm1, xmm2, xmm3			; 0F 24 4B 323 10 /or/ 0F 24 4B 332 18
pshaq xmm1, xmm2, [0]			; 0F 24 4B 026 10 00 00
pshaq xmm1, xmm2, dqword [0]		; 0F 24 4B 026 10 00 00
pshaq xmm1, [0], xmm3			; 0F 24 4B 036 18 00 00
pshaq xmm1, dqword [0], xmm3		; 0F 24 4B 036 18 00 00

pshaw xmm1, xmm2, xmm3			; 0F 24 49 323 10 /or/ 0F 24 49 332 18
pshaw xmm1, xmm2, [0]			; 0F 24 49 026 10 00 00
pshaw xmm1, xmm2, dqword [0]		; 0F 24 49 026 10 00 00
pshaw xmm1, [0], xmm3			; 0F 24 49 036 18 00 00
pshaw xmm1, dqword [0], xmm3		; 0F 24 49 036 18 00 00

pshlb xmm1, xmm2, xmm3			; 0F 24 44 323 10 /or/ 0F 24 44 332 18
pshlb xmm1, xmm2, [0]			; 0F 24 44 026 10 00 00
pshlb xmm1, xmm2, dqword [0]		; 0F 24 44 026 10 00 00
pshlb xmm1, [0], xmm3			; 0F 24 44 036 18 00 00
pshlb xmm1, dqword [0], xmm3		; 0F 24 44 036 18 00 00

pshld xmm1, xmm2, xmm3			; 0F 24 46 323 10 /or/ 0F 24 46 332 18
pshld xmm1, xmm2, [0]			; 0F 24 46 026 10 00 00
pshld xmm1, xmm2, dqword [0]		; 0F 24 46 026 10 00 00
pshld xmm1, [0], xmm3			; 0F 24 46 036 18 00 00
pshld xmm1, dqword [0], xmm3		; 0F 24 46 036 18 00 00

pshlq xmm1, xmm2, xmm3			; 0F 24 47 323 10 /or/ 0F 24 47 332 18
pshlq xmm1, xmm2, [0]			; 0F 24 47 026 10 00 00
pshlq xmm1, xmm2, dqword [0]		; 0F 24 47 026 10 00 00
pshlq xmm1, [0], xmm3			; 0F 24 47 036 18 00 00
pshlq xmm1, dqword [0], xmm3		; 0F 24 47 036 18 00 00

pshlw xmm1, xmm2, xmm3			; 0F 24 45 323 10 /or/ 0F 24 45 332 18
pshlw xmm1, xmm2, [0]			; 0F 24 45 026 10 00 00
pshlw xmm1, xmm2, dqword [0]		; 0F 24 45 026 10 00 00
pshlw xmm1, [0], xmm3			; 0F 24 45 036 18 00 00
pshlw xmm1, dqword [0], xmm3		; 0F 24 45 036 18 00 00

; SSE5 instructions that are also SSE4.1 instructions

ptest xmm1, xmm2			; 66 0F 38 17 312
ptest xmm1, [0]				; 66 0F 38 17 016 00 00
ptest xmm1, dqword [0]			; 66 0F 38 17 016 00 00

roundpd xmm1, xmm2, 5			; 66 0F 3A 09 312 05
roundpd xmm1, [0], byte 5		; 66 0F 3A 09 016 00 00 05
roundpd xmm1, dqword [0], 5		; 66 0F 3A 09 016 00 00 05

roundps xmm1, xmm2, 5			; 66 0F 3A 08 312 05
roundps xmm1, [0], byte 5		; 66 0F 3A 08 016 00 00 05
roundps xmm1, dqword [0], 5		; 66 0F 3A 08 016 00 00 05

roundsd xmm1, xmm2, 5			; 66 0F 3A 0B 312 05
roundsd xmm1, [0], byte 5		; 66 0F 3A 0B 016 00 00 05
roundsd xmm1, qword [0], 5		; 66 0F 3A 0B 016 00 00 05

roundss xmm1, xmm2, 5			; 66 0F 3A 0A 312 05
roundss xmm1, [0], byte 5		; 66 0F 3A 0A 016 00 00 05
roundss xmm1, dword [0], 5		; 66 0F 3A 0A 016 00 00 05

