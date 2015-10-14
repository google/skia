[bits 64]
addpd xmm1, xmm2
addpd xmm1, dqword [rbx]

addps xmm1, xmm2
addps xmm1, dqword [rbx]

addsd xmm1, xmm2
addsd xmm1, qword [rbx]

addss xmm1, xmm2
addss xmm1, dword [rbx]

addsubpd xmm1, xmm2
addsubpd xmm1, dqword [rbx]

addsubps xmm1, xmm2
addsubps xmm1, dqword [rbx]

andnpd xmm1, xmm2
andnpd xmm1, dqword [rbx]

andnps xmm1, xmm2
andnps xmm1, dqword [rbx]

andpd xmm1, xmm2
andpd xmm1, dqword [rbx]

andps xmm1, xmm2
andps xmm1, dqword [rbx]

cmppd xmm1, xmm2, 0
cmppd xmm1, dqword [rbx], 0
cmpeqpd xmm1, xmm2
cmpeqpd xmm1, dqword [rbx]

cmpps xmm1, xmm2, 0
cmpps xmm1, dqword [rbx], 0
cmpeqps xmm1, xmm2
cmpeqps xmm1, dqword [rbx]

cmpsd xmm1, xmm2, 0
cmpsd xmm1, qword [rbx], 0
cmpeqsd xmm1, xmm2
cmpeqsd xmm1, qword [rbx]

cmpss xmm1, xmm2, 0
cmpss xmm1, dword [rbx], 0
cmpeqss xmm1, xmm2
cmpeqss xmm1, dword [rbx]

comisd xmm1, xmm2
comisd xmm1, qword [rbx]

comiss xmm1, xmm2
comiss xmm1, dword [rbx]

cvtdq2pd xmm1, xmm2
cvtdq2pd xmm1, qword [rbx]

cvtdq2ps xmm1, xmm2
cvtdq2ps xmm1, dqword [rbx]

cvtpd2dq xmm1, xmm2
cvtpd2dq xmm1, dqword [rbx]

cvtpd2pi mm1, xmm2		; mmx
cvtpd2pi mm1, dqword [rbx]

cvtpd2ps xmm1, xmm2
cvtpd2ps xmm1, dqword [rbx]

cvtpi2pd xmm1, mm2		; mmx
cvtpi2pd xmm1, qword [rbx]

cvtpi2ps xmm1, mm2		; mmx
cvtpi2ps xmm1, qword [rbx]

cvtps2dq xmm1, xmm2
cvtps2dq xmm1, dqword [rbx]

cvtps2pd xmm1, xmm2
cvtps2pd xmm1, qword [rbx]

cvtps2pi mm1, xmm2
cvtps2pi mm1, qword [rbx]

cvtsd2si rbx, xmm2
cvtsd2si rbx, qword [rbx]

cvtsd2ss xmm1, xmm2
cvtsd2ss xmm1, qword [rbx]

cvtsi2sd xmm1, ebx
cvtsi2sd xmm1, dword [rbx]
cvtsi2sd xmm1, rbx
cvtsi2sd xmm1, qword [rbx]

cvtsi2ss xmm1, ebx
cvtsi2ss xmm1, dword [rbx]
cvtsi2ss xmm1, rbx
cvtsi2ss xmm1, qword [rbx]

cvtss2sd xmm1, xmm2
cvtss2sd xmm1, dword [rbx]

cvtss2si ebx, xmm2
cvtss2si ebx, dword [rbx]
cvtss2si rbx, xmm2
cvtss2si rbx, dword [rbx]

cvttpd2dq xmm1, xmm2
cvttpd2dq xmm1, dqword [rbx]

cvttpd2pi mm1, xmm2
cvttpd2pi mm1, dqword [rbx]

cvttps2dq xmm1, xmm2
cvttps2dq xmm1, dqword [rbx]

cvttps2pi mm1, xmm2
cvttps2pi mm1, qword [rbx]

cvttsd2si eax, xmm1
cvttsd2si eax, qword [rbx]
cvttsd2si rax, xmm1
cvttsd2si rax, qword [rbx]

cvttss2si eax, xmm1
cvttss2si eax, dword [rbx]
cvttss2si rax, xmm1
cvttss2si rax, dword [rbx]

divpd xmm1, xmm2
divpd xmm1, dqword [rbx]

divps xmm1, xmm2
divps xmm1, dqword [rbx]

divsd xmm1, xmm2
divsd xmm1, qword [rbx]

divss xmm1, xmm2
divss xmm1, dword [rbx]

extrq xmm1, 0, 1
extrq xmm1, byte 0, byte 1
extrq xmm1, xmm2

haddpd xmm1, xmm2
haddpd xmm1, dqword [rbx]

haddps xmm1, xmm2
haddps xmm1, dqword [rbx]

hsubpd xmm1, xmm2
hsubpd xmm1, dqword [rbx]

hsubps xmm1, xmm2
hsubps xmm1, dqword [rbx]

insertq xmm1, xmm2, 0, 1
insertq xmm1, xmm2, byte 0, byte 1
insertq xmm1, xmm2

lddqu xmm1, dqword [rbx]

ldmxcsr dword [rbx]

maskmovdqu xmm1, xmm2

maxpd xmm1, xmm2
maxpd xmm1, dqword [rbx]

maxps xmm1, xmm2
maxps xmm1, dqword [rbx]

maxsd xmm1, xmm2
maxsd xmm1, qword [rbx]

maxss xmm1, xmm2
maxss xmm1, dword [rbx]

minpd xmm1, xmm2
minpd xmm1, dqword [rbx]

minps xmm1, xmm2
minps xmm1, dqword [rbx]

minsd xmm1, xmm2
minsd xmm1, qword [rbx]

minss xmm1, xmm2
minss xmm1, dword [rbx]

movapd xmm1, xmm2
movapd xmm1, dqword [rbx]
movapd dqword [rbx], xmm2

movaps xmm1, xmm2
movaps xmm1, dqword [rbx]
movaps dqword [rbx], xmm2

movd xmm1, ebx
movd xmm1, dword [rbx]
movd xmm1, rbx
movd xmm1, qword [rbx]
movd dword [rbx], xmm2
movd qword [rbx], xmm2

movddup xmm1, xmm2
movddup xmm1, qword [rbx]

movdq2q mm1, xmm2

movdqa xmm1, xmm2
movdqa xmm1, dqword [rbx]
movdqa dqword [rbx], xmm2

movdqu xmm1, xmm2
movdqu xmm1, dqword [rbx]
movdqu dqword [rbx], xmm2

movhlps xmm1, xmm2

movhpd xmm1, qword [rbx]
movhpd qword [rbx], xmm2

movhps xmm1, qword [rbx]
movhps qword [rbx], xmm2

movlhps xmm1, xmm2

movlpd xmm1, qword [rbx]
movlpd qword [rbx], xmm2

movlps xmm1, qword [rbx]
movlps qword [rbx], xmm2

movmskpd ebx, xmm2

movmskps ebx, xmm2

movntdq dqword [rbx], xmm2

movntpd dqword [rbx], xmm2

movntps dqword [rbx], xmm2

movntsd qword [rbx], xmm2

movntss dword [rbx], xmm2

movq xmm1, xmm2
movq xmm1, qword [rbx]
movq qword [rbx], xmm2

movq2dq xmm1, mm2

movsd xmm1, xmm2
movsd xmm1, qword [rbx]
movsd qword [rbx], xmm2

movshdup xmm1, xmm2
movshdup xmm1, dqword [rbx]

movsldup xmm1, xmm2
movsldup xmm1, dqword [rbx]

movss xmm1, xmm2
movss xmm1, dword [rbx]
movss dword [rbx], xmm2

movupd xmm1, xmm2
movupd xmm1, dqword [rbx]
movupd dqword [rbx], xmm2

movups xmm1, xmm2
movups xmm1, dqword [rbx]
movups dqword [rbx], xmm2

mulpd xmm1, xmm2
mulpd xmm1, dqword [rbx]

mulps xmm1, xmm2
mulps xmm1, dqword [rbx]

mulsd xmm1, xmm2
mulsd xmm1, qword [rbx]

mulss xmm1, xmm2
mulss xmm1, dword [rbx]

orpd xmm1, xmm2
orpd xmm1, dqword [rbx]

orps xmm1, xmm2
orps xmm1, dqword [rbx]

packssdw xmm1, xmm2
packssdw xmm1, dqword [rbx]

packsswb xmm1, xmm2
packsswb xmm1, dqword [rbx]

packuswb xmm1, xmm2
packuswb xmm1, dqword [rbx]

paddb xmm1, xmm2
paddb xmm1, dqword [rbx]

paddd xmm1, xmm2
paddd xmm1, dqword [rbx]

paddq xmm1, xmm2
paddq xmm1, dqword [rbx]

paddsb xmm1, xmm2
paddsb xmm1, dqword [rbx]

paddsw xmm1, xmm2
paddsw xmm1, dqword [rbx]

paddusb xmm1, xmm2
paddusb xmm1, dqword [rbx]

paddusw xmm1, xmm2
paddusw xmm1, dqword [rbx]

paddw xmm1, xmm2
paddw xmm1, dqword [rbx]

pand xmm1, xmm2
pand xmm1, dqword [rbx]

pandn xmm1, xmm2
pandn xmm1, dqword [rbx]

pavgb xmm1, xmm2
pavgb xmm1, dqword [rbx]

pavgw xmm1, xmm2
pavgw xmm1, dqword [rbx]

pcmpeqb xmm1, xmm2
pcmpeqb xmm1, dqword [rbx]

pcmpeqd xmm1, xmm2
pcmpeqd xmm1, dqword [rbx]

pcmpeqw xmm1, xmm2
pcmpeqw xmm1, dqword [rbx]

pcmpgtb xmm1, xmm2
pcmpgtb xmm1, dqword [rbx]

pcmpgtd xmm1, xmm2
pcmpgtd xmm1, dqword [rbx]

pcmpgtw xmm1, xmm2
pcmpgtw xmm1, dqword [rbx]

pextrw ebx, xmm2, byte 0

pinsrw xmm1, ebx, byte 0
pinsrw xmm1, word [rbx], byte 0

pmaddwd xmm1, xmm2
pmaddwd xmm1, dqword [rbx]

pmaxsw xmm1, xmm2
pmaxsw xmm1, dqword [rbx]

pmaxub xmm1, xmm2
pmaxub xmm1, dqword [rbx]

pminsw xmm1, xmm2
pminsw xmm1, dqword [rbx]

pminub xmm1, xmm2
pminub xmm1, dqword [rbx]

pmovmskb eax, xmm2

pmulhuw xmm1, xmm2
pmulhuw xmm1, dqword [rbx]

pmulhw xmm1, xmm2
pmulhw xmm1, dqword [rbx]

pmullw xmm1, xmm2
pmullw xmm1, dqword [rbx]

pmuludq xmm1, xmm2
pmuludq xmm1, dqword [rbx]

por xmm1, xmm2
por xmm1, dqword [rbx]

psadbw xmm1, xmm2
psadbw xmm1, dqword [rbx]

pshufd xmm1, xmm2, byte 0
pshufd xmm1, dqword [rbx], byte 0

pshufhw xmm1, xmm2, byte 0
pshufhw xmm1, dqword [rbx], byte 0

pshuflw xmm1, xmm2, byte 0
pshuflw xmm1, dqword [rbx], byte 0

pslld xmm1, xmm2
pslld xmm1, dqword [rbx]
pslld xmm1, byte 5

pslldq xmm1, byte 5

psllq xmm1, xmm2
psllq xmm1, dqword [rbx]
psllq xmm1, byte 5

psllw xmm1, xmm2
psllw xmm1, dqword [rbx]
psllw xmm1, byte 5

psrad xmm1, xmm2
psrad xmm1, dqword [rbx]
psrad xmm1, byte 5

psraw xmm1, xmm2
psraw xmm1, dqword [rbx]
psraw xmm1, byte 5

psrld xmm1, xmm2
psrld xmm1, dqword [rbx]
psrld xmm1, byte 5

psrldq xmm1, byte 5

psrlq xmm1, xmm2
psrlq xmm1, dqword [rbx]
psrlq xmm1, byte 5

psrlw xmm1, xmm2
psrlw xmm1, dqword [rbx]
psrlw xmm1, byte 5

psubb xmm1, xmm2
psubb xmm1, dqword [rbx]

psubd xmm1, xmm2
psubd xmm1, dqword [rbx]

psubq xmm1, xmm2
psubq xmm1, dqword [rbx]

psubsb xmm1, xmm2
psubsb xmm1, dqword [rbx]

psubsw xmm1, xmm2
psubsw xmm1, dqword [rbx]

psubusb xmm1, xmm2
psubusb xmm1, dqword [rbx]

psubusw xmm1, xmm2
psubusw xmm1, dqword [rbx]

psubw xmm1, xmm2
psubw xmm1, dqword [rbx]

punpckhbw xmm1, xmm2
punpckhbw xmm1, dqword [rbx]

punpckhdq xmm1, xmm2
punpckhdq xmm1, dqword [rbx]

punpckhqdq xmm1, xmm2
punpckhqdq xmm1, dqword [rbx]

punpckhwd xmm1, xmm2
punpckhwd xmm1, dqword [rbx]

punpcklbw xmm1, xmm2
punpcklbw xmm1, dqword [rbx]

punpckldq xmm1, xmm2
punpckldq xmm1, dqword [rbx]

punpcklqdq xmm1, xmm2
punpcklqdq xmm1, dqword [rbx]

punpcklwd xmm1, xmm2
punpcklwd xmm1, dqword [rbx]

pxor xmm1, xmm2
pxor xmm1, dqword [rbx]

rcpps xmm1, xmm2
rcpps xmm1, dqword [rbx]

rcpss xmm1, xmm2
rcpss xmm1, dword [rbx]

rsqrtps xmm1, xmm2
rsqrtps xmm1, dqword [rbx]

rsqrtss xmm1, xmm2
rsqrtss xmm1, dword [rbx]

shufpd xmm1, xmm2, 0
shufpd xmm1, dqword [rbx], byte 0

shufps xmm1, xmm2, 0
shufps xmm1, dqword [rbx], byte 0

sqrtpd xmm1, xmm2
sqrtpd xmm1, dqword [rbx]

sqrtps xmm1, xmm2
sqrtps xmm1, dqword [rbx]

sqrtsd xmm1, xmm2
sqrtsd xmm1, qword [rbx]

sqrtss xmm1, xmm2
sqrtss xmm1, dword [rbx]

stmxcsr dword [rbx]

subpd xmm1, xmm2
subpd xmm1, dqword [rbx]

subps xmm1, xmm2
subps xmm1, dqword [rbx]

subsd xmm1, xmm2
subsd xmm1, qword [rbx]

subss xmm1, xmm2
subss xmm1, dword [rbx]

ucomisd xmm1, xmm2
ucomisd xmm1, qword [rbx]

ucomiss xmm1, xmm2
ucomiss xmm1, dword [rbx]

unpckhpd xmm1, xmm2
unpckhpd xmm1, dqword [rbx]

unpckhps xmm1, xmm2
unpckhps xmm1, dqword [rbx]

unpcklpd xmm1, xmm2
unpcklpd xmm1, dqword [rbx]

unpcklps xmm1, xmm2
unpcklps xmm1, dqword [rbx]

xorpd xmm1, xmm2
xorpd xmm1, dqword [rbx]

xorps xmm1, xmm2
xorps xmm1, dqword [rbx]

