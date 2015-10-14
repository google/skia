[bits 32]
addsubpd xmm5, xmm7
addsubpd xmm0, [eax]
addsubps xmm1, xmm5
addsubps xmm3, dqword [edx]
fisttp word [0]
fisttp dword [4]
fisttp qword [8]
haddpd xmm2, xmm4
haddpd xmm7, [ecx+4]
haddps xmm6, xmm1
haddps xmm0, dqword [0]
hsubpd xmm5, xmm3
hsubpd xmm1, [ebp]
hsubps xmm4, xmm1
hsubps xmm2, [esp]
lddqu xmm3, [ecx+edx*4+8]
monitor
movddup xmm7, xmm6
movddup xmm1, qword [4]
movshdup xmm3, xmm4
movshdup xmm2, [0]
movsldup xmm0, xmm7
movsldup xmm5, dqword [eax+ebx]
mwait
