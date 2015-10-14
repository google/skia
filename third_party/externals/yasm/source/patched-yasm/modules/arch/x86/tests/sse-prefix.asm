[bits 32]
movsw
es movsw
rep movsw
rep fs movsw
fs rep movsw

movsd
es movsd
rep movsd
rep fs movsd
fs rep movsd

cmpss xmm0, [eax], 0
cmpss xmm0, [es:eax], 0

cmpsd
fs cmpsd
rep fs cmpsd
fs rep cmpsd

cmpsd xmm0, [eax], 0
cmpsd xmm0, [es:eax], 0

[bits 64]
movsw
rep movsw

movsd
rep movsd

cmpss xmm0, [r8], 0

cmpsd
rep cmpsd

cmpsd xmm0, [r8], 0


